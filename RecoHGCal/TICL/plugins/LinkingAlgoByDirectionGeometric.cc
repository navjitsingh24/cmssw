#include <cmath>
#include <string>
#include "RecoHGCal/TICL/plugins/LinkingAlgoByDirectionGeometric.h"

#include "DataFormats/GeometrySurface/interface/BoundDisk.h"
#include "DataFormats/HGCalReco/interface/Common.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"

#include "RecoParticleFlow/PFProducer/interface/PFMuonAlgo.h"

using namespace ticl;

LinkingAlgoByDirectionGeometric::LinkingAlgoByDirectionGeometric(const edm::ParameterSet &conf)
    : LinkingAlgoBase(conf),
      del_tk_ts_layer1_(conf.getParameter<double>("delta_tk_ts_layer1")),
      del_tk_ts_int_(conf.getParameter<double>("delta_tk_ts_interface")),
      del_ts_em_had_(conf.getParameter<double>("delta_ts_em_had")),
      del_ts_had_had_(conf.getParameter<double>("delta_ts_had_had")),
      timing_quality_threshold_(conf.getParameter<double>("track_time_quality_threshold")),
      pid_threshold_(conf.getParameter<double>("pid_threshold")),
      energy_em_over_total_threshold_(conf.getParameter<double>("energy_em_over_total_threshold")),
      filter_on_categories_(conf.getParameter<std::vector<int>>("filter_hadronic_on_categories")),
      cutTk_(conf.getParameter<std::string>("cutTk")) {}

LinkingAlgoByDirectionGeometric::~LinkingAlgoByDirectionGeometric() {}

void LinkingAlgoByDirectionGeometric::initialize(const HGCalDDDConstants *hgcons,
                                                 const hgcal::RecHitTools rhtools,
                                                 const edm::ESHandle<MagneticField> bfieldH,
                                                 const edm::ESHandle<Propagator> propH) {
  hgcons_ = hgcons;
  rhtools_ = rhtools;
  buildLayers();

  bfield_ = bfieldH;
  propagator_ = propH;
}

math::XYZVector LinkingAlgoByDirectionGeometric::propagateTrackster(const Trackster &t,
                                                                    const unsigned idx,
                                                                    float zVal,
                                                                    std::array<TICLLayerTile, 2> &tracksterTiles) {
  // needs only the positive Z co-ordinate of the surface to propagate to
  // the correct sign is calculated inside according to the barycenter of trackster
  Vector const &baryc = t.barycenter();
  Vector directnv = t.eigenvectors(0);

  // barycenter as direction for tracksters w/ poor PCA
  // propagation still done to get the cartesian coords
  // which are anyway converted to eta, phi in linking
  // -> can be simplified later

  //FP: disable PCA propagation for the moment and fallback to barycenter position
  // if (t.eigenvalues()[0] / t.eigenvalues()[1] < 20)
  directnv = baryc.unit();

  zVal *= (baryc.Z() > 0) ? 1 : -1;

  double par = (zVal - baryc.Z()) / directnv.Z();
  double xOnSurface = par * directnv.X() + baryc.X();
  double yOnSurface = par * directnv.Y() + baryc.Y();
  Vector tPoint(xOnSurface, yOnSurface, zVal);
  if (tPoint.Eta() > 0)
    tracksterTiles[1].fill(tPoint.Eta(), tPoint.Phi(), idx);

  else if (tPoint.Eta() < 0)
    tracksterTiles[0].fill(tPoint.Eta(), tPoint.Phi(), idx);

  return tPoint;
}

void LinkingAlgoByDirectionGeometric::findTrackstersInWindow(
    const std::vector<std::pair<Vector, unsigned>> &seedingCollection,
    const std::array<TICLLayerTile, 2> &tracksterTiles,
    double delta,
    unsigned trackstersSize,
    std::vector<std::vector<unsigned>> &resultCollection,
    bool useMask = false) {
  // Finds tracksters in tracksterTiles within an eta-phi window
  // (given by delta) of the objects (track/trackster) in the seedingCollection.
  // Element i in resultCollection is the vector of trackster
  // indices found close to the i-th object in the seedingCollection.
  // If specified, Tracksters are masked once found as close to an object.
  std::vector<int> mask(trackstersSize, 0);

  for (auto &i : seedingCollection) {
    auto seed_eta = i.first.Eta();
    auto seed_phi = i.first.Phi();
    unsigned seedId = i.second;
    auto sideZ = seed_eta > 0;  //forward or backward region
    const TICLLayerTile &tile = tracksterTiles[sideZ];
    double eta_min = std::max(abs(seed_eta) - delta, (double)TileConstants::minEta);
    double eta_max = std::min(abs(seed_eta) + delta, (double)TileConstants::maxEta);

    std::array<int, 4> search_box = tile.searchBoxEtaPhi(eta_min, eta_max, seed_phi - delta, seed_phi + delta);
    if (search_box[2] > search_box[3])
      search_box[3] += TileConstants::nPhiBins;

    for (int eta_i = search_box[0]; eta_i <= search_box[1]; ++eta_i) {
      for (int phi_i = search_box[2]; phi_i <= search_box[3]; ++phi_i) {
        const auto &in_box = tile[tile.globalBin(eta_i, (phi_i % TileConstants::nPhiBins))];
        for (const unsigned t_i : in_box) {
          if (!mask[t_i]) {
            resultCollection[seedId].push_back(t_i);
            if (useMask)
              mask[t_i] = 1;
          }
        }
      }
    }
  }  // seeding collection loop
}

bool LinkingAlgoByDirectionGeometric::timeAndEnergyCompatible(double &total_raw_energy,
                                                              const reco::Track &track,
                                                              const Trackster &trackster,
                                                              const float &tkT,
                                                              const float &tkTErr,
                                                              const float &tkTimeQual) {
  double threshold = std::min(0.2 * trackster.raw_energy(), 10.0);

  bool energyCompatible = (total_raw_energy + trackster.raw_energy() < track.p() + threshold);
  // compatible if trackster time is within 3sigma of
  // track time; compatible if either: no time assigned
  // to trackster or track time quality is below threshold
  double tsT = trackster.time();
  double tsTErr = trackster.timeError();

  bool timeCompatible = false;

  if (tsT == -99. or tkTimeQual < timing_quality_threshold_)
    timeCompatible = true;
  else {
    timeCompatible = (std::abs(tsT - tkT) < maxDeltaT_ * sqrt(tsTErr * tsTErr + tkTErr * tkTErr));
  }

  if (LinkingAlgoBase::algo_verbosity_ > VerbosityLevel::Advanced) {
    if (!(energyCompatible))
      LogDebug("LinkingAlgoByDirectionGeometric")
          << "energy incompatible : track p " << track.p() << " trackster energy " << trackster.raw_energy() << "\n";
    if (!(timeCompatible))
      LogDebug("LinkingAlgoByDirectionGeometric") << "time incompatible : track time " << tkT << " +/- " << tkTErr
                                                  << " trackster time " << tsT << " +/- " << tsTErr << "\n";
  }
  return energyCompatible && timeCompatible;
}

void LinkingAlgoByDirectionGeometric::recordTrackster(const unsigned ts,  //trackster index
                                                      const std::vector<Trackster> &tracksters,
                                                      const edm::Handle<std::vector<Trackster>> tsH,
                                                      std::vector<unsigned> &ts_mask,
                                                      double &energy_in_candidate,
                                                      TICLCandidate &candidate) {
  if (ts_mask[ts])
    return;
  candidate.addTrackster(edm::Ptr<Trackster>(tsH, ts));
  ts_mask[ts] = 1;
  energy_in_candidate += tracksters[ts].raw_energy();
}

void LinkingAlgoByDirectionGeometric::dumpLinksFound(std::vector<std::vector<unsigned>> &resultCollection,
                                                     const char *label) const {
#ifdef EDM_ML_DEBUG
  if (!(LinkingAlgoBase::algo_verbosity_ > VerbosityLevel::Advanced))
    return;

  LogDebug("LinkingAlgoByDirectionGeometric") << "All links found - " << label << "\n";
  LogDebug("LinkingAlgoByDirectionGeometric") << "(seed can either be a track or trackster depending on the step)\n";
  for (unsigned i = 0; i < resultCollection.size(); ++i) {
    LogDebug("LinkingAlgoByDirectionGeometric") << "seed " << i << " - tracksters : ";
    const auto &links = resultCollection[i];
    for (unsigned j = 0; j < links.size(); ++j) {
      LogDebug("LinkingAlgoByDirectionGeometric") << j;
    }
    LogDebug("LinkingAlgoByDirectionGeometric") << "\n";
  }
#endif  // EDM_ML_DEBUG
}

void LinkingAlgoByDirectionGeometric::buildLayers() {
  // build disks at HGCal front & EM-Had interface for track propagation

  float zVal = hgcons_->waferZ(1, true);
  std::pair<double, double> rMinMax = hgcons_->rangeR(zVal, true);

  float zVal_interface = rhtools_.getPositionLayer(rhtools_.lastLayerEE()).z();
  std::pair<double, double> rMinMax_interface = hgcons_->rangeR(zVal_interface, true);

  for (int iSide = 0; iSide < 2; ++iSide) {
    float zSide = (iSide == 0) ? (-1. * zVal) : zVal;
    firstDisk_[iSide] =
        std::make_unique<GeomDet>(Disk::build(Disk::PositionType(0, 0, zSide),
                                              Disk::RotationType(),
                                              SimpleDiskBounds(rMinMax.first, rMinMax.second, zSide - 0.5, zSide + 0.5))
                                      .get());

    zSide = (iSide == 0) ? (-1. * zVal_interface) : zVal_interface;
    interfaceDisk_[iSide] = std::make_unique<GeomDet>(
        Disk::build(Disk::PositionType(0, 0, zSide),
                    Disk::RotationType(),
                    SimpleDiskBounds(rMinMax_interface.first, rMinMax_interface.second, zSide - 0.5, zSide + 0.5))
            .get());
  }
}

void LinkingAlgoByDirectionGeometric::linkTracksters(const edm::Handle<std::vector<reco::Track>> tkH,
                                                     const edm::ValueMap<float> &tkTime,
                                                     const edm::ValueMap<float> &tkTimeErr,
                                                     const edm::ValueMap<float> &tkTimeQual,
                                                     const std::vector<reco::Muon> &muons,
                                                     const edm::Handle<std::vector<Trackster>> tsH,
                                                     std::vector<TICLCandidate> &resultLinked) {
  const auto &tracks = *tkH;
  const auto &tracksters = *tsH;

  auto bFieldProd = bfield_.product();
  const Propagator &prop = (*propagator_);

  // propagated point collections
  // elements in the propagated points collecions are used
  // to look for potential linkages in the appropriate tiles
  std::vector<std::pair<Vector, unsigned>> trackPColl;     // propagated track points and index of track in collection
  std::vector<std::pair<Vector, unsigned>> tkPropIntColl;  // tracks propagated to lastLayerEE
  std::vector<std::pair<Vector, unsigned>> tsPropIntColl;  // Tracksters in CE-E, propagated to lastLayerEE
  std::vector<std::pair<Vector, unsigned>> tsHadPropIntColl;  // Tracksters in CE-H, propagated to lastLayerEE
  trackPColl.reserve(tracks.size());
  tkPropIntColl.reserve(tracks.size());
  tsPropIntColl.reserve(tracksters.size());
  tsHadPropIntColl.reserve(tracksters.size());
  // tiles, element 0 is bw, 1 is fw
  std::array<TICLLayerTile, 2> tracksterPropTiles = {};  // all Tracksters, propagated to layer 1
  std::array<TICLLayerTile, 2> tsPropIntTiles = {};      // all Tracksters, propagated to lastLayerEE
  std::array<TICLLayerTile, 2> tsHadPropIntTiles = {};   // Tracksters in CE-H, propagated to lastLayerEE

  // filters (true for) anything but EM
  auto isHadron = [&](const Trackster &t) -> bool {
    auto cumulative_prob = 0.;
    for (const auto index : filter_on_categories_) {
      cumulative_prob += t.id_probabilities(index);
    }
    return ((cumulative_prob <= pid_threshold_) and (t.raw_em_energy() != t.raw_energy())) or
           (t.raw_em_energy() < energy_em_over_total_threshold_ * t.raw_energy());
  };
  if (LinkingAlgoBase::algo_verbosity_ > VerbosityLevel::Advanced)
    LogDebug("LinkingAlgoByDirectionGeometric") << "------- Geometric Linking ------- \n";

  // Propagate tracks
  std::vector<unsigned> candidateTrackIds;
  candidateTrackIds.reserve(tracks.size());
  for (unsigned i = 0; i < tracks.size(); ++i) {
    const auto &tk = tracks[i];

    reco::TrackRef trackref = reco::TrackRef(tkH, i);
    // also veto tracks associated to muons
    int muId = PFMuonAlgo::muAssocToTrack(trackref, muons);
    if (LinkingAlgoBase::algo_verbosity_ > VerbosityLevel::Advanced)
      LogDebug("LinkingAlgoByDirectionGeometric")
          << "track " << i << " - eta " << tk.eta() << " phi " << tk.phi() << " time " << tkTime[reco::TrackRef(tkH, i)]
          << " time qual " << tkTimeQual[reco::TrackRef(tkH, i)] << "  muid " << muId << "\n";
    if (!cutTk_((tk)) or muId != -1)
      continue;

    // record tracks that can used to make a ticlcandidate
    candidateTrackIds.push_back(i);

    // don't consider tracks below 2 GeV for linking
    if (std::sqrt(tk.p() * tk.p() + ticl::mpion2) < tkEnergyCut_)
      continue;

    int iSide = int(tk.eta() > 0);
    const auto &fts = trajectoryStateTransform::outerFreeState((tk), bFieldProd);
    // to the HGCal front
    const auto &tsos = prop.propagate(fts, firstDisk_[iSide]->surface());
    if (tsos.isValid()) {
      Vector trackP(tsos.globalPosition().x(), tsos.globalPosition().y(), tsos.globalPosition().z());
      trackPColl.emplace_back(trackP, i);
    }
    // to lastLayerEE
    const auto &tsos_int = prop.propagate(fts, interfaceDisk_[iSide]->surface());
    if (tsos_int.isValid()) {
      Vector trackP(tsos_int.globalPosition().x(), tsos_int.globalPosition().y(), tsos_int.globalPosition().z());
      tkPropIntColl.emplace_back(trackP, i);
    }
  }  // Tracks
  tkPropIntColl.shrink_to_fit();
  trackPColl.shrink_to_fit();
  candidateTrackIds.shrink_to_fit();
  // Propagate tracksters
  for (unsigned i = 0; i < tracksters.size(); ++i) {
    const auto &t = tracksters[i];
    if (LinkingAlgoBase::algo_verbosity_ > VerbosityLevel::Advanced)
      LogDebug("LinkingAlgoByDirectionGeometric")
          << "trackster " << i << " - eta " << t.barycenter().eta() << " phi " << t.barycenter().phi() << " time "
          << t.time() << " energy " << t.raw_energy() << "\n";

    // to HGCal front
    float zVal = hgcons_->waferZ(1, true);
    auto tsP = propagateTrackster(t, i, zVal, tracksterPropTiles);

    // to lastLayerEE
    zVal = rhtools_.getPositionLayer(rhtools_.lastLayerEE()).z();
    tsP = propagateTrackster(t, i, zVal, tsPropIntTiles);

    if (!isHadron(t))  // EM tracksters
      tsPropIntColl.emplace_back(tsP, i);
    else {  // HAD
      tsHadPropIntTiles[(t.barycenter().Z() > 0) ? 1 : 0].fill(tsP.Eta(), tsP.Phi(), i);
      tsHadPropIntColl.emplace_back(tsP, i);
    }
  }  // TS
  tsPropIntColl.shrink_to_fit();
  tsHadPropIntColl.shrink_to_fit();
  // Track - Trackster link finding
  // step 3: tracks -> all tracksters, at layer 1

  std::vector<std::vector<unsigned>> tsNearTk(tracks.size());
  findTrackstersInWindow(trackPColl, tracksterPropTiles, del_tk_ts_layer1_, tracksters.size(), tsNearTk);

  // step 4: tracks -> all tracksters, at lastLayerEE

  std::vector<std::vector<unsigned>> tsNearTkAtInt(tracks.size());
  findTrackstersInWindow(tkPropIntColl, tsPropIntTiles, del_tk_ts_int_, tracksters.size(), tsNearTkAtInt);

  // Trackster - Trackster link finding
  // step 2: tracksters EM -> HAD, at lastLayerEE

  std::vector<std::vector<unsigned>> tsNearAtInt(tracksters.size());
  findTrackstersInWindow(tsPropIntColl, tsHadPropIntTiles, del_ts_em_had_, tracksters.size(), tsNearAtInt, true);

  // step 1: tracksters HAD -> HAD, at lastLayerEE

  std::vector<std::vector<unsigned>> tsHadNearAtInt(tracksters.size());
  findTrackstersInWindow(tsHadPropIntColl, tsHadPropIntTiles, del_ts_had_had_, tracksters.size(), tsHadNearAtInt, true);
#ifdef EDM_ML_DEBUG

  dumpLinksFound(tsNearTk, "track -> tracksters at layer 1");
  dumpLinksFound(tsNearTkAtInt, "track -> tracksters at lastLayerEE");
  dumpLinksFound(tsNearAtInt, "EM -> HAD tracksters at lastLayerEE");
  dumpLinksFound(tsHadNearAtInt, "HAD -> HAD tracksters at lastLayerEE");
#endif  //EDM_ML_DEBUG
  // make final collections

  std::vector<TICLCandidate> chargedCandidates;
  std::vector<TICLCandidate> chargedHadronsFromTk;
  std::vector<unsigned int> chargedMask(tracksters.size(), 0);
  for (unsigned &i : candidateTrackIds) {
    if (tsNearTk[i].empty() && tsNearTkAtInt[i].empty()) {  // nothing linked to track, make charged hadrons
      TICLCandidate chargedHad;
      const auto &tk = tracks[i];
      chargedHad.setCharge(tk.charge());
      chargedHad.setPdgId(211 * tk.charge());
      chargedHad.setTrackPtr(edm::Ptr<reco::Track>(tkH, i));
      float energy = std::sqrt(tk.p() * tk.p() + ticl::mpion2);
      chargedHad.setRawEnergy(energy);
      math::PtEtaPhiMLorentzVector p4Polar(tk.pt(), tk.eta(), tk.phi(), ticl::mpion);
      chargedHad.setP4(p4Polar);
      chargedHadronsFromTk.push_back(chargedHad);
      continue;
    }

    TICLCandidate chargedCandidate;
    double total_raw_energy = 0.;

    auto tkRef = reco::TrackRef(tkH, i);
    auto track_time = tkTime[tkRef];
    auto track_timeErr = tkTimeErr[tkRef];
    auto track_timeQual = tkTimeQual[tkRef];

    for (const unsigned ts3_idx : tsNearTk[i]) {  // tk -> ts
      if (timeAndEnergyCompatible(
              total_raw_energy, tracks[i], tracksters[ts3_idx], track_time, track_timeErr, track_timeQual)) {
        recordTrackster(ts3_idx, tracksters, tsH, chargedMask, total_raw_energy, chargedCandidate);
      }
      for (const unsigned ts2_idx : tsNearAtInt[ts3_idx]) {  // ts_EM -> ts_HAD
        if (timeAndEnergyCompatible(
                total_raw_energy, tracks[i], tracksters[ts2_idx], track_time, track_timeErr, track_timeQual)) {
          recordTrackster(ts2_idx, tracksters, tsH, chargedMask, total_raw_energy, chargedCandidate);
        }
        for (const unsigned ts1_idx : tsHadNearAtInt[ts2_idx]) {  // ts_HAD -> ts_HAD
          if (timeAndEnergyCompatible(
                  total_raw_energy, tracks[i], tracksters[ts1_idx], track_time, track_timeErr, track_timeQual)) {
            recordTrackster(ts1_idx, tracksters, tsH, chargedMask, total_raw_energy, chargedCandidate);
          }
        }
      }
      for (const unsigned ts1_idx : tsHadNearAtInt[ts3_idx]) {  // ts_HAD -> ts_HAD
        if (timeAndEnergyCompatible(
                total_raw_energy, tracks[i], tracksters[ts1_idx], track_time, track_timeErr, track_timeQual)) {
          recordTrackster(ts1_idx, tracksters, tsH, chargedMask, total_raw_energy, chargedCandidate);
        }
      }
    }
    for (const unsigned ts4_idx : tsNearTkAtInt[i]) {  // do the same for tk -> ts links at the interface
      if (timeAndEnergyCompatible(
              total_raw_energy, tracks[i], tracksters[ts4_idx], track_time, track_timeErr, track_timeQual)) {
        recordTrackster(ts4_idx, tracksters, tsH, chargedMask, total_raw_energy, chargedCandidate);
      }
      for (const unsigned ts2_idx : tsNearAtInt[ts4_idx]) {
        if (timeAndEnergyCompatible(
                total_raw_energy, tracks[i], tracksters[ts2_idx], track_time, track_timeErr, track_timeQual)) {
          recordTrackster(ts2_idx, tracksters, tsH, chargedMask, total_raw_energy, chargedCandidate);
        }
        for (const unsigned ts1_idx : tsHadNearAtInt[ts2_idx]) {
          if (timeAndEnergyCompatible(
                  total_raw_energy, tracks[i], tracksters[ts1_idx], track_time, track_timeErr, track_timeQual)) {
            recordTrackster(ts1_idx, tracksters, tsH, chargedMask, total_raw_energy, chargedCandidate);
          }
        }
      }
      for (const unsigned ts1_idx : tsHadNearAtInt[ts4_idx]) {
        if (timeAndEnergyCompatible(
                total_raw_energy, tracks[i], tracksters[ts1_idx], track_time, track_timeErr, track_timeQual)) {
          recordTrackster(ts1_idx, tracksters, tsH, chargedMask, total_raw_energy, chargedCandidate);
        }
      }
    }

    // do not create a candidate if no tracksters were added to candidate
    // can happen if all the tracksters linked to that track were already masked
    if (!chargedCandidate.tracksters().empty()) {
      chargedCandidate.setTrackPtr(edm::Ptr<reco::Track>(tkH, i));
      chargedCandidates.push_back(chargedCandidate);
    } else {  // create charged hadron
      TICLCandidate chargedHad;
      const auto &tk = tracks[i];
      chargedHad.setCharge(tk.charge());
      chargedHad.setPdgId(211 * tk.charge());
      chargedHad.setTrackPtr(edm::Ptr<reco::Track>(tkH, i));
      float energy = std::sqrt(tk.p() * tk.p() + ticl::mpion2);
      chargedHad.setRawEnergy(energy);
      math::PtEtaPhiMLorentzVector p4Polar(tk.pt(), tk.eta(), tk.phi(), mpion);
      chargedHad.setP4(p4Polar);
      chargedHadronsFromTk.push_back(chargedHad);
    }
  }

  std::vector<TICLCandidate> neutralCandidates;
  std::vector<int> neutralMask(tracksters.size(), 0);
  for (unsigned i = 0; i < tracksters.size(); ++i) {
    if (chargedMask[i])
      continue;

    TICLCandidate neutralCandidate;
    if (tsNearAtInt[i].empty() && tsHadNearAtInt[i].empty() && !neutralMask[i]) {  // nothing linked to this ts
      neutralCandidate.addTrackster(edm::Ptr<Trackster>(tsH, i));
      neutralMask[i] = 1;
      neutralCandidates.push_back(neutralCandidate);
      continue;
    }
    if (!neutralMask[i]) {
      neutralCandidate.addTrackster(edm::Ptr<Trackster>(tsH, i));
      neutralMask[i] = 1;
    }
    for (const unsigned ts2_idx : tsNearAtInt[i]) {
      if (chargedMask[ts2_idx])
        continue;
      if (!neutralMask[ts2_idx]) {
        neutralCandidate.addTrackster(edm::Ptr<Trackster>(tsH, ts2_idx));
        neutralMask[ts2_idx] = 1;
      }
      for (const unsigned ts1_idx : tsHadNearAtInt[ts2_idx]) {
        if (chargedMask[ts1_idx])
          continue;
        if (!neutralMask[ts1_idx]) {
          neutralCandidate.addTrackster(edm::Ptr<Trackster>(tsH, ts1_idx));
          neutralMask[ts1_idx] = 1;
        }
      }
    }
    for (const unsigned ts1_idx : tsHadNearAtInt[i]) {
      if (chargedMask[ts1_idx])
        continue;
      if (!neutralMask[ts1_idx]) {
        neutralCandidate.addTrackster(edm::Ptr<Trackster>(tsH, ts1_idx));
        neutralMask[ts1_idx] = 1;
      }
    }
    // filter empty candidates
    if (!neutralCandidate.tracksters().empty()) {
      neutralCandidates.push_back(neutralCandidate);
    }
  }

  // set other attributes of created candidates
  for (auto &cand : chargedCandidates) {
    bool isHAD = false;
    double rawE = 0.;
    const auto track = cand.trackPtr();
    for (const auto &ts : cand.tracksters()) {
      // isHAD if atleast one trackster is not EM
      if (isHadron(*ts))
        isHAD = true;
      rawE += ts->raw_energy();
    }
    auto pdgID = isHAD ? 211 : 11;

    cand.setCharge(track->charge());
    cand.setPdgId(pdgID * track->charge());
    cand.setRawEnergy(rawE);
    math::XYZTLorentzVector p4(rawE * track->momentum().unit().x(),
                               rawE * track->momentum().unit().y(),
                               rawE * track->momentum().unit().z(),
                               rawE);
    cand.setP4(p4);
  }

  for (auto &cand : neutralCandidates) {
    bool isHAD = false;
    double rawE = 0.;
    const auto track = cand.trackPtr();
    double wtSum_baryc[3] = {0};
    for (const auto &ts : cand.tracksters()) {
      if (isHadron(*ts))
        isHAD = true;
      rawE += ts->raw_energy();
      wtSum_baryc[0] += (ts->raw_energy()) * (ts->barycenter().x());
      wtSum_baryc[1] += (ts->raw_energy()) * (ts->barycenter().y());
      wtSum_baryc[2] += (ts->raw_energy()) * (ts->barycenter().z());
    }
    Vector combined_baryc(wtSum_baryc[0] / rawE, wtSum_baryc[1] / rawE, wtSum_baryc[2] / rawE);
    auto pdgID = isHAD ? 130 : 22;
    auto const &magnitude = isHAD ? std::sqrt(rawE * rawE - ticl::mpion2) : rawE;

    cand.setCharge(0);
    cand.setPdgId(pdgID);
    cand.setRawEnergy(rawE);
    math::XYZTLorentzVector p4(magnitude * combined_baryc.unit().x(),
                               magnitude * combined_baryc.unit().y(),
                               magnitude * combined_baryc.unit().z(),
                               rawE);

    cand.setP4(p4);
  }

  resultLinked.insert(std::end(resultLinked), std::begin(neutralCandidates), std::end(neutralCandidates));
  resultLinked.insert(std::end(resultLinked), std::begin(chargedCandidates), std::end(chargedCandidates));
  resultLinked.insert(std::end(resultLinked), std::begin(chargedHadronsFromTk), std::end(chargedHadronsFromTk));

}  // linkTracksters

void LinkingAlgoByDirectionGeometric::fillPSetDescription(edm::ParameterSetDescription &desc) {
  desc.add<std::string>("cutTk",
                        "1.48 < abs(eta) < 3.0 && pt > 1. && quality(\"highPurity\") && "
                        "hitPattern().numberOfLostHits(\"MISSING_OUTER_HITS\") < 5");
  desc.add<double>("delta_tk_ts_layer1", 0.02);
  desc.add<double>("delta_tk_ts_interface", 0.03);
  desc.add<double>("delta_ts_em_had", 0.03);
  desc.add<double>("delta_ts_had_had", 0.03);
  desc.add<double>("track_time_quality_threshold", 0.5);
  desc.add<double>("pid_threshold", 0.5);
  desc.add<double>("energy_em_over_total_threshold", 0.9);
  desc.add<std::vector<int>>("filter_hadronic_on_categories", {0, 1});
  LinkingAlgoBase::fillPSetDescription(desc);
}
