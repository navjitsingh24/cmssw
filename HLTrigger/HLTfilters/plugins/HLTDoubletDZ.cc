#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "HLTDoubletDZ.h"

#include "DataFormats/Common/interface/Handle.h"

#include "DataFormats/Common/interface/Ref.h"
#include "DataFormats/HLTReco/interface/TriggerFilterObjectWithRefs.h"

//#include "DataFormats/Candidate/interface/Particle.h"
#include "DataFormats/RecoCandidate/interface/RecoEcalCandidate.h"
#include "DataFormats/RecoCandidate/interface/RecoEcalCandidateFwd.h"
#include "DataFormats/EgammaCandidates/interface/Electron.h"
#include "DataFormats/EgammaCandidates/interface/ElectronFwd.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidate.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidateFwd.h"
#include "DataFormats/L1TMuonPhase2/interface/TrackerMuon.h"
#include "DataFormats/L1TParticleFlow/interface/PFTau.h"
#include "DataFormats/L1TParticleFlow/interface/HPSPFTau.h"
#include "DataFormats/L1TParticleFlow/interface/HPSPFTauFwd.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "FWCore/Framework/interface/MakerMacros.h"

#include "DataFormats/Math/interface/deltaR.h"

#include "HLTrigger/HLTcore/interface/defaultModuleLabel.h"
#include <cmath>

//
// constructors and destructor
//
template <typename T1, typename T2>
HLTDoubletDZ<T1, T2>::HLTDoubletDZ(const edm::ParameterSet& iConfig)
    : HLTFilter(iConfig),
      originTag1_(iConfig.template getParameter<std::vector<edm::InputTag>>("originTag1")),
      originTag2_(iConfig.template getParameter<std::vector<edm::InputTag>>("originTag2")),
      inputTag1_(iConfig.template getParameter<edm::InputTag>("inputTag1")),
      inputTag2_(iConfig.template getParameter<edm::InputTag>("inputTag2")),
      inputToken1_(consumes<trigger::TriggerFilterObjectWithRefs>(inputTag1_)),
      inputToken2_(consumes<trigger::TriggerFilterObjectWithRefs>(inputTag2_)),
      //electronToken_ (consumes<reco::ElectronCollection>(iConfig.template getParameter<edm::InputTag>("electronTag"))),
      triggerType1_(iConfig.template getParameter<int>("triggerType1")),
      triggerType2_(iConfig.template getParameter<int>("triggerType2")),
      minDR_(iConfig.template getParameter<double>("MinDR")),
      maxDZ_(iConfig.template getParameter<double>("MaxDZ")),
      minPixHitsForDZ_(iConfig.template getParameter<int>("MinPixHitsForDZ")),
      min_N_(iConfig.template getParameter<int>("MinN")),
      checkSC_(iConfig.template getParameter<bool>("checkSC")),
      same_(inputTag1_.encode() == inputTag2_.encode())  // same collections to be compared?
{}

template <>
HLTDoubletDZ<reco::RecoEcalCandidate, reco::RecoEcalCandidate>::HLTDoubletDZ(const edm::ParameterSet& iConfig)
    : HLTFilter(iConfig),
      originTag1_(iConfig.template getParameter<std::vector<edm::InputTag>>("originTag1")),
      originTag2_(iConfig.template getParameter<std::vector<edm::InputTag>>("originTag2")),
      inputTag1_(iConfig.template getParameter<edm::InputTag>("inputTag1")),
      inputTag2_(iConfig.template getParameter<edm::InputTag>("inputTag2")),
      inputToken1_(consumes<trigger::TriggerFilterObjectWithRefs>(inputTag1_)),
      inputToken2_(consumes<trigger::TriggerFilterObjectWithRefs>(inputTag2_)),
      electronToken_(consumes<reco::ElectronCollection>(iConfig.template getParameter<edm::InputTag>("electronTag"))),
      triggerType1_(iConfig.template getParameter<int>("triggerType1")),
      triggerType2_(iConfig.template getParameter<int>("triggerType2")),
      minDR_(iConfig.template getParameter<double>("MinDR")),
      maxDZ_(iConfig.template getParameter<double>("MaxDZ")),
      minPixHitsForDZ_(iConfig.template getParameter<int>("MinPixHitsForDZ")),
      min_N_(iConfig.template getParameter<int>("MinN")),
      checkSC_(iConfig.template getParameter<bool>("checkSC")),
      same_(inputTag1_.encode() == inputTag2_.encode())  // same collections to be compared?
{}

template <>
HLTDoubletDZ<reco::RecoChargedCandidate, reco::RecoEcalCandidate>::HLTDoubletDZ(const edm::ParameterSet& iConfig)
    : HLTFilter(iConfig),
      originTag1_(iConfig.template getParameter<std::vector<edm::InputTag>>("originTag1")),
      originTag2_(iConfig.template getParameter<std::vector<edm::InputTag>>("originTag2")),
      inputTag1_(iConfig.template getParameter<edm::InputTag>("inputTag1")),
      inputTag2_(iConfig.template getParameter<edm::InputTag>("inputTag2")),
      inputToken1_(consumes<trigger::TriggerFilterObjectWithRefs>(inputTag1_)),
      inputToken2_(consumes<trigger::TriggerFilterObjectWithRefs>(inputTag2_)),
      electronToken_(consumes<reco::ElectronCollection>(iConfig.template getParameter<edm::InputTag>("electronTag"))),
      triggerType1_(iConfig.template getParameter<int>("triggerType1")),
      triggerType2_(iConfig.template getParameter<int>("triggerType2")),
      minDR_(iConfig.template getParameter<double>("MinDR")),
      maxDZ_(iConfig.template getParameter<double>("MaxDZ")),
      minPixHitsForDZ_(iConfig.template getParameter<int>("MinPixHitsForDZ")),
      min_N_(iConfig.template getParameter<int>("MinN")),
      checkSC_(iConfig.template getParameter<bool>("checkSC")),
      same_(inputTag1_.encode() == inputTag2_.encode())  // same collections to be compared?
{}

template <>
HLTDoubletDZ<reco::RecoEcalCandidate, reco::RecoChargedCandidate>::HLTDoubletDZ(const edm::ParameterSet& iConfig)
    : HLTFilter(iConfig),
      originTag1_(iConfig.template getParameter<std::vector<edm::InputTag>>("originTag1")),
      originTag2_(iConfig.template getParameter<std::vector<edm::InputTag>>("originTag2")),
      inputTag1_(iConfig.template getParameter<edm::InputTag>("inputTag1")),
      inputTag2_(iConfig.template getParameter<edm::InputTag>("inputTag2")),
      inputToken1_(consumes<trigger::TriggerFilterObjectWithRefs>(inputTag1_)),
      inputToken2_(consumes<trigger::TriggerFilterObjectWithRefs>(inputTag2_)),
      electronToken_(consumes<reco::ElectronCollection>(iConfig.template getParameter<edm::InputTag>("electronTag"))),
      triggerType1_(iConfig.template getParameter<int>("triggerType1")),
      triggerType2_(iConfig.template getParameter<int>("triggerType2")),
      minDR_(iConfig.template getParameter<double>("MinDR")),
      maxDZ_(iConfig.template getParameter<double>("MaxDZ")),
      minPixHitsForDZ_(iConfig.template getParameter<int>("MinPixHitsForDZ")),
      min_N_(iConfig.template getParameter<int>("MinN")),
      checkSC_(iConfig.template getParameter<bool>("checkSC")),
      same_(inputTag1_.encode() == inputTag2_.encode())  // same collections to be compared?
{}

template <>
HLTDoubletDZ<reco::RecoChargedCandidate, reco::RecoChargedCandidate>::HLTDoubletDZ(const edm::ParameterSet& iConfig)
    : HLTFilter(iConfig),
      originTag1_(iConfig.template getParameter<std::vector<edm::InputTag>>("originTag1")),
      originTag2_(iConfig.template getParameter<std::vector<edm::InputTag>>("originTag2")),
      inputTag1_(iConfig.template getParameter<edm::InputTag>("inputTag1")),
      inputTag2_(iConfig.template getParameter<edm::InputTag>("inputTag2")),
      inputToken1_(consumes<trigger::TriggerFilterObjectWithRefs>(inputTag1_)),
      inputToken2_(consumes<trigger::TriggerFilterObjectWithRefs>(inputTag2_)),
      //electronToken_ (consumes<reco::ElectronCollection>(iConfig.template getParameter<edm::InputTag>("electronTag"))),
      triggerType1_(iConfig.template getParameter<int>("triggerType1")),
      triggerType2_(iConfig.template getParameter<int>("triggerType2")),
      minDR_(iConfig.template getParameter<double>("MinDR")),
      maxDZ_(iConfig.template getParameter<double>("MaxDZ")),
      minPixHitsForDZ_(iConfig.template getParameter<int>("MinPixHitsForDZ")),
      min_N_(iConfig.template getParameter<int>("MinN")),
      checkSC_(iConfig.template getParameter<bool>("checkSC")),
      same_(inputTag1_.encode() == inputTag2_.encode())  // same collections to be compared?
{}

template <>
HLTDoubletDZ<l1t::TrackerMuon, l1t::TrackerMuon>::HLTDoubletDZ(const edm::ParameterSet& iConfig)
    : HLTFilter(iConfig),
      originTag1_(iConfig.template getParameter<std::vector<edm::InputTag>>("originTag1")),
      originTag2_(iConfig.template getParameter<std::vector<edm::InputTag>>("originTag2")),
      inputTag1_(iConfig.template getParameter<edm::InputTag>("inputTag1")),
      inputTag2_(iConfig.template getParameter<edm::InputTag>("inputTag2")),
      inputToken1_(consumes<trigger::TriggerFilterObjectWithRefs>(inputTag1_)),
      inputToken2_(consumes<trigger::TriggerFilterObjectWithRefs>(inputTag2_)),
      triggerType1_(iConfig.template getParameter<int>("triggerType1")),
      triggerType2_(iConfig.template getParameter<int>("triggerType2")),
      minDR_(iConfig.template getParameter<double>("MinDR")),
      maxDZ_(iConfig.template getParameter<double>("MaxDZ")),
      minPixHitsForDZ_(iConfig.template getParameter<int>("MinPixHitsForDZ")),
      min_N_(iConfig.template getParameter<int>("MinN")),
      checkSC_(iConfig.template getParameter<bool>("checkSC")),
      same_(inputTag1_.encode() == inputTag2_.encode())  // same collections to be compared?
{}

template <typename T1, typename T2>
HLTDoubletDZ<T1, T2>::~HLTDoubletDZ() {}

template <typename T1, typename T2>
void HLTDoubletDZ<T1, T2>::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  makeHLTFilterDescription(desc);
  std::vector<edm::InputTag> originTag1(1, edm::InputTag("hltOriginal1"));
  std::vector<edm::InputTag> originTag2(1, edm::InputTag("hltOriginal2"));
  desc.add<std::vector<edm::InputTag>>("originTag1", originTag1);
  desc.add<std::vector<edm::InputTag>>("originTag2", originTag2);
  desc.add<edm::InputTag>("inputTag1", edm::InputTag("hltFiltered1"));
  desc.add<edm::InputTag>("inputTag2", edm::InputTag("hltFiltered2"));
  desc.add<int>("triggerType1", 0);
  desc.add<int>("triggerType2", 0);
  desc.add<double>("MinDR", -1.0);
  desc.add<double>("MaxDZ", 0.2);
  desc.add<int>("MinPixHitsForDZ", 0);
  desc.add<bool>("checkSC", false);
  desc.add<int>("MinN", 1);
  descriptions.add(defaultModuleLabel<HLTDoubletDZ<T1, T2>>(), desc);
}

template <>
void HLTDoubletDZ<reco::RecoEcalCandidate, reco::RecoEcalCandidate>::fillDescriptions(
    edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  makeHLTFilterDescription(desc);
  std::vector<edm::InputTag> originTag1(1, edm::InputTag("hltOriginal1"));
  std::vector<edm::InputTag> originTag2(1, edm::InputTag("hltOriginal2"));
  desc.add<std::vector<edm::InputTag>>("originTag1", originTag1);
  desc.add<std::vector<edm::InputTag>>("originTag2", originTag2);
  desc.add<edm::InputTag>("inputTag1", edm::InputTag("hltFiltered1"));
  desc.add<edm::InputTag>("inputTag2", edm::InputTag("hltFiltered2"));
  desc.add<edm::InputTag>("electronTag", edm::InputTag("electronTag"));
  desc.add<int>("triggerType1", 0);
  desc.add<int>("triggerType2", 0);
  desc.add<double>("MinDR", -1.0);
  desc.add<double>("MaxDZ", 0.2);
  desc.add<int>("MinPixHitsForDZ", 0);
  desc.add<bool>("checkSC", false);
  desc.add<int>("MinN", 1);
  descriptions.add(defaultModuleLabel<HLTDoubletDZ<reco::RecoEcalCandidate, reco::RecoEcalCandidate>>(), desc);
}

template <>
void HLTDoubletDZ<reco::RecoChargedCandidate, reco::RecoEcalCandidate>::fillDescriptions(
    edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  makeHLTFilterDescription(desc);
  std::vector<edm::InputTag> originTag1(1, edm::InputTag("hltOriginal1"));
  std::vector<edm::InputTag> originTag2(1, edm::InputTag("hltOriginal2"));
  desc.add<std::vector<edm::InputTag>>("originTag1", originTag1);
  desc.add<std::vector<edm::InputTag>>("originTag2", originTag2);
  desc.add<edm::InputTag>("inputTag1", edm::InputTag("hltFiltered1"));
  desc.add<edm::InputTag>("inputTag2", edm::InputTag("hltFiltered2"));
  desc.add<edm::InputTag>("electronTag", edm::InputTag("electronTag"));
  desc.add<int>("triggerType1", 0);
  desc.add<int>("triggerType2", 0);
  desc.add<double>("MinDR", -1.0);
  desc.add<double>("MaxDZ", 0.2);
  desc.add<int>("MinPixHitsForDZ", 0);
  desc.add<bool>("checkSC", false);
  desc.add<int>("MinN", 1);
  descriptions.add(defaultModuleLabel<HLTDoubletDZ<reco::RecoChargedCandidate, reco::RecoEcalCandidate>>(), desc);
}

template <>
void HLTDoubletDZ<reco::RecoEcalCandidate, reco::RecoChargedCandidate>::fillDescriptions(
    edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  makeHLTFilterDescription(desc);
  std::vector<edm::InputTag> originTag1(1, edm::InputTag("hltOriginal1"));
  std::vector<edm::InputTag> originTag2(1, edm::InputTag("hltOriginal2"));
  desc.add<std::vector<edm::InputTag>>("originTag1", originTag1);
  desc.add<std::vector<edm::InputTag>>("originTag2", originTag2);
  desc.add<edm::InputTag>("inputTag1", edm::InputTag("hltFiltered1"));
  desc.add<edm::InputTag>("inputTag2", edm::InputTag("hltFiltered2"));
  desc.add<edm::InputTag>("electronTag", edm::InputTag("electronTag"));
  desc.add<int>("triggerType1", 0);
  desc.add<int>("triggerType2", 0);
  desc.add<double>("MinDR", -1.0);
  desc.add<double>("MaxDZ", 0.2);
  desc.add<int>("MinPixHitsForDZ", 0);
  desc.add<bool>("checkSC", false);
  desc.add<int>("MinN", 1);
  descriptions.add(defaultModuleLabel<HLTDoubletDZ<reco::RecoEcalCandidate, reco::RecoChargedCandidate>>(), desc);
}

template <>
void HLTDoubletDZ<reco::RecoChargedCandidate, reco::RecoChargedCandidate>::fillDescriptions(
    edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  makeHLTFilterDescription(desc);
  std::vector<edm::InputTag> originTag1(1, edm::InputTag("hltOriginal1"));
  std::vector<edm::InputTag> originTag2(1, edm::InputTag("hltOriginal2"));
  desc.add<std::vector<edm::InputTag>>("originTag1", originTag1);
  desc.add<std::vector<edm::InputTag>>("originTag2", originTag2);
  desc.add<edm::InputTag>("inputTag1", edm::InputTag("hltFiltered1"));
  desc.add<edm::InputTag>("inputTag2", edm::InputTag("hltFiltered2"));
  desc.add<int>("triggerType1", 0);
  desc.add<int>("triggerType2", 0);
  desc.add<double>("MinDR", -1.0);
  desc.add<double>("MaxDZ", 0.2);
  desc.add<int>("MinPixHitsForDZ", 0);
  desc.add<bool>("checkSC", false);
  desc.add<int>("MinN", 1);
  descriptions.add(defaultModuleLabel<HLTDoubletDZ<reco::RecoChargedCandidate, reco::RecoChargedCandidate>>(), desc);
}

template <typename T1, typename T2>
bool
//HLTDoubletDZ<T1, T2>::getCollections(edm::Event& iEvent, std::vector<T1Ref>& coll1, std::vector<T2Ref>& coll2) const {
HLTDoubletDZ<T1, T2>::getCollections(edm::Event& iEvent,
                                     std::vector<T1Ref>& coll1,
                                     std::vector<T2Ref>& coll2,
                                     trigger::TriggerFilterObjectWithRefs& filterproduct) const {
  edm::Handle<trigger::TriggerFilterObjectWithRefs> handle1, handle2;
  if (iEvent.getByToken(inputToken1_, handle1) and iEvent.getByToken(inputToken2_, handle2)) {
    // get hold of pre-filtered object collections
    handle1->getObjects(triggerType1_, coll1);
    handle2->getObjects(triggerType2_, coll2);
    const trigger::size_type n1(coll1.size());
    const trigger::size_type n2(coll2.size());

    if (saveTags()) {
      edm::InputTag tagOld;
      for (unsigned int i = 0; i < originTag1_.size(); ++i) {
        filterproduct.addCollectionTag(originTag1_[i]);
      }
      tagOld = edm::InputTag();
      for (trigger::size_type i1 = 0; i1 != n1; ++i1) {
        const edm::ProductID pid(coll1[i1].id());
        const auto& prov = iEvent.getStableProvenance(pid);
        const std::string& label(prov.moduleLabel());
        const std::string& instance(prov.productInstanceName());
        const std::string& process(prov.processName());
        edm::InputTag tagNew(edm::InputTag(label, instance, process));
        if (tagOld.encode() != tagNew.encode()) {
          filterproduct.addCollectionTag(tagNew);
          tagOld = tagNew;
        }
      }
      for (unsigned int i = 0; i < originTag2_.size(); ++i) {
        filterproduct.addCollectionTag(originTag2_[i]);
      }
      tagOld = edm::InputTag();
      for (trigger::size_type i2 = 0; i2 != n2; ++i2) {
        const edm::ProductID pid(coll2[i2].id());
        const auto& prov = iEvent.getStableProvenance(pid);
        const std::string& label(prov.moduleLabel());
        const std::string& instance(prov.productInstanceName());
        const std::string& process(prov.processName());
        edm::InputTag tagNew(edm::InputTag(label, instance, process));
        if (tagOld.encode() != tagNew.encode()) {
          filterproduct.addCollectionTag(tagNew);
          tagOld = tagNew;
        }
      }
    }

    return true;
  } else
    return false;
}

template <typename T1, typename T2>
bool HLTDoubletDZ<T1, T2>::computeDZ(edm::Event& iEvent, T1Ref& r1, T2Ref& r2) const {
  const reco::Candidate& candidate1(*r1);
  const reco::Candidate& candidate2(*r2);
  if (reco::deltaR(candidate1, candidate2) < minDR_)
    return false;
  if (std::abs(candidate1.vz() - candidate2.vz()) > maxDZ_)
    return false;

  return true;
}

template <>
bool HLTDoubletDZ<reco::RecoEcalCandidate, reco::RecoChargedCandidate>::computeDZ(edm::Event& iEvent,
                                                                                  T1Ref& r1,
                                                                                  T2Ref& r2) const {
  edm::Handle<reco::ElectronCollection> electronHandle_;
  iEvent.getByToken(electronToken_, electronHandle_);
  if (!electronHandle_.isValid())
    edm::LogError("HLTDoubletDZ") << "HLTDoubletDZ: Electron Handle not valid.";

  if (reco::deltaR(*r1, *r2) < minDR_)
    return false;
  reco::Electron e1;
  for (auto const& eleIt : *electronHandle_) {
    if (eleIt.superCluster() == r1->superCluster())
      e1 = eleIt;
  }

  const reco::RecoChargedCandidate& candidate2(*r2);
  bool skipDZ = false;
  if (minPixHitsForDZ_ > 0 && (e1.gsfTrack()->hitPattern().numberOfValidPixelHits() < minPixHitsForDZ_ ||
                               candidate2.track()->hitPattern().numberOfValidPixelHits() < minPixHitsForDZ_))
    skipDZ = true;
  if (!skipDZ && std::abs(e1.vz() - candidate2.vz()) > maxDZ_)
    return false;

  return true;
}

template <>
bool HLTDoubletDZ<reco::RecoChargedCandidate, reco::RecoEcalCandidate>::computeDZ(edm::Event& iEvent,
                                                                                  T1Ref& r1,
                                                                                  T2Ref& r2) const {
  edm::Handle<reco::ElectronCollection> electronHandle_;
  iEvent.getByToken(electronToken_, electronHandle_);
  if (!electronHandle_.isValid())
    edm::LogError("HLTDoubletDZ") << "HLTDoubletDZ: Electron Handle not valid.";

  if (reco::deltaR(*r1, *r2) < minDR_)
    return false;
  reco::Electron e2;
  for (auto const& eleIt : *electronHandle_) {
    if (eleIt.superCluster() == r2->superCluster())
      e2 = eleIt;
  }

  const reco::RecoChargedCandidate& candidate1(*r1);
  bool skipDZ = false;
  if (minPixHitsForDZ_ > 0 && (candidate1.track()->hitPattern().numberOfValidPixelHits() < minPixHitsForDZ_ ||
                               e2.gsfTrack()->hitPattern().numberOfValidPixelHits() < minPixHitsForDZ_))
    skipDZ = true;
  if (!skipDZ && std::abs(e2.vz() - candidate1.vz()) > maxDZ_)
    return false;

  return true;
}

template <>
bool HLTDoubletDZ<reco::RecoEcalCandidate, reco::RecoEcalCandidate>::computeDZ(edm::Event& iEvent,
                                                                               T1Ref& r1,
                                                                               T2Ref& r2) const {
  edm::Handle<reco::ElectronCollection> electronHandle_;
  iEvent.getByToken(electronToken_, electronHandle_);
  if (!electronHandle_.isValid())
    edm::LogError("HLTDoubletDZ") << "HLTDoubletDZ: Electron Handle not valid.";

  if (reco::deltaR(*r1, *r2) < minDR_)
    return false;
  reco::Electron e1, e2;
  for (auto const& eleIt : *electronHandle_) {
    if (eleIt.superCluster() == r2->superCluster())
      e2 = eleIt;
    if (eleIt.superCluster() == r1->superCluster())
      e1 = eleIt;
  }

  bool skipDZ = false;
  if (minPixHitsForDZ_ > 0 && (e1.gsfTrack()->hitPattern().numberOfValidPixelHits() < minPixHitsForDZ_ ||
                               e2.gsfTrack()->hitPattern().numberOfValidPixelHits() < minPixHitsForDZ_))
    skipDZ = true;
  if (!skipDZ && std::abs(e2.vz() - e1.vz()) > maxDZ_)
    return false;

  return true;
}

template <>
bool HLTDoubletDZ<reco::RecoChargedCandidate, reco::RecoChargedCandidate>::computeDZ(edm::Event& iEvent,
                                                                                     T1Ref& r1,
                                                                                     T2Ref& r2) const {
  const reco::RecoChargedCandidate& candidate1(*r1);
  const reco::RecoChargedCandidate& candidate2(*r2);
  if (reco::deltaR(candidate1, candidate2) < minDR_)
    return false;
  bool skipDZ = false;
  if (minPixHitsForDZ_ > 0 && (candidate1.track()->hitPattern().numberOfValidPixelHits() < minPixHitsForDZ_ ||
                               candidate2.track()->hitPattern().numberOfValidPixelHits() < minPixHitsForDZ_))
    skipDZ = true;
  if (!skipDZ && std::abs(candidate1.vz() - candidate2.vz()) > maxDZ_)
    return false;

  return true;
}

template <>
bool HLTDoubletDZ<l1t::TrackerMuon, l1t::TrackerMuon>::computeDZ(edm::Event& iEvent,
                                                                 l1t::TrackerMuonRef& r1,
                                                                 l1t::TrackerMuonRef& r2) const {
  // We don't care about minPixHitsForDZ_ with the L1TTrackerMuons,
  // especially because the pixel does not participate in the L1T
  if (std::abs(r1->phZ0() - r2->phZ0()) > maxDZ_)
    return false;

  if (reco::deltaR2(r1->phEta(), r1->phPhi(), r2->phEta(), r2->phPhi()) < minDR_ * minDR_)
    return false;

  return true;
}

template <>
bool HLTDoubletDZ<l1t::HPSPFTau, l1t::HPSPFTau>::computeDZ(edm::Event& iEvent,
                                                           l1t::HPSPFTauRef& r1,
                                                           l1t::HPSPFTauRef& r2) const {
  const l1t::HPSPFTau& candidate1(*r1);
  const l1t::HPSPFTau& candidate2(*r2);
  if (reco::deltaR(candidate1, candidate2) < minDR_)
    return false;

  // We don't care about minPixHitsForDZ_ with the L1HPSPFTaus,
  // especially because the pixel does not participate in the L1T
  if (std::abs(candidate1.leadChargedPFCand()->pfTrack()->vertex().z() -
               candidate2.leadChargedPFCand()->pfTrack()->vertex().z()) > maxDZ_)
    return false;

  return true;
}

// ------------ method called to produce the data  ------------
template <typename T1, typename T2>
bool HLTDoubletDZ<T1, T2>::hltFilter(edm::Event& iEvent,
                                     const edm::EventSetup& iSetup,
                                     trigger::TriggerFilterObjectWithRefs& filterproduct) const {
  // All HLT filters must create and fill an HLT filter object,
  // recording any reconstructed physics objects satisfying (or not)
  // this HLT filter, and place it in the Event.
  bool accept(false);

  std::vector<T1Ref> coll1;
  std::vector<T2Ref> coll2;

  if (getCollections(iEvent, coll1, coll2, filterproduct)) {
    int n(0);
    T1Ref r1;
    T2Ref r2;

    for (unsigned int i1 = 0; i1 != coll1.size(); i1++) {
      r1 = coll1[i1];
      //const reco::Candidate& candidate1(*r1);
      unsigned int I(0);
      if (same_) {
        I = i1 + 1;
      }
      for (unsigned int i2 = I; i2 != coll2.size(); i2++) {
        r2 = coll2[i2];
        if (checkSC_) {
          if (r1->superCluster().isNonnull() && r2->superCluster().isNonnull()) {
            if (r1->superCluster() == r2->superCluster())
              continue;
          }
        }

        if (!computeDZ(iEvent, r1, r2))
          continue;

        n++;
        filterproduct.addObject(triggerType1_, r1);
        filterproduct.addObject(triggerType2_, r2);
      }
    }

    accept = accept || (n >= min_N_);
  }

  return accept;
}

/// Special instantiation for L1TTrackerMuon
/// L1TTrackerMuon are *not* RecoCandidates, therefore they don't implement superCluster()
/// They are LeafCandidates instead
template <>
bool HLTDoubletDZ<l1t::TrackerMuon, l1t::TrackerMuon>::hltFilter(
    edm::Event& iEvent, const edm::EventSetup& iSetup, trigger::TriggerFilterObjectWithRefs& filterproduct) const {
  // All HLT filters must create and fill an HLT filter object,
  // recording any reconstructed physics objects satisfying (or not)
  // this HLT filter, and place it in the Event.
  bool accept(false);

  std::vector<l1t::TrackerMuonRef> coll1;
  std::vector<l1t::TrackerMuonRef> coll2;

  if (getCollections(iEvent, coll1, coll2, filterproduct)) {
    int n(0);
    l1t::TrackerMuonRef r1;
    l1t::TrackerMuonRef r2;

    for (unsigned int i1 = 0; i1 != coll1.size(); i1++) {
      r1 = coll1[i1];
      unsigned int I(0);
      if (same_) {
        I = i1 + 1;
      }
      for (unsigned int i2 = I; i2 != coll2.size(); i2++) {
        r2 = coll2[i2];

        if (!computeDZ(iEvent, r1, r2))
          continue;

        n++;
        filterproduct.addObject(triggerType1_, r1);
        filterproduct.addObject(triggerType2_, r2);
      }
    }

    accept = accept || (n >= min_N_);
  }

  return accept;
}

/// Special instantiation for L1PFTau
/// L1PFTau are *not* RecoCandidates, therefore they don't implement superCluster()
/// They are LeafCandidates instead
template <>
bool HLTDoubletDZ<l1t::PFTau, l1t::PFTau>::hltFilter(edm::Event& iEvent,
                                                     const edm::EventSetup& iSetup,
                                                     trigger::TriggerFilterObjectWithRefs& filterproduct) const {
  // All HLT filters must create and fill an HLT filter object,
  // recording any reconstructed physics objects satisfying (or not)
  // this HLT filter, and place it in the Event.
  bool accept(false);

  std::vector<l1t::PFTauRef> coll1;
  std::vector<l1t::PFTauRef> coll2;

  if (getCollections(iEvent, coll1, coll2, filterproduct)) {
    int n(0);
    l1t::PFTauRef r1;
    l1t::PFTauRef r2;

    for (unsigned int i1 = 0; i1 != coll1.size(); i1++) {
      r1 = coll1[i1];
      unsigned int I(0);
      if (same_) {
        I = i1 + 1;
      }
      for (unsigned int i2 = I; i2 != coll2.size(); i2++) {
        r2 = coll2[i2];

        if (!computeDZ(iEvent, r1, r2))
          continue;

        n++;
        filterproduct.addObject(triggerType1_, r1);
        filterproduct.addObject(triggerType2_, r2);
      }
    }

    accept = accept || (n >= min_N_);
  }

  return accept;
}

/// Special instantiation for L1HPSPFTau
/// L1HPSPFTau are *not* RecoCandidates, therefore they don't implement superCluster()
/// They are LeafCandidates instead
template <>
bool HLTDoubletDZ<l1t::HPSPFTau, l1t::HPSPFTau>::hltFilter(edm::Event& iEvent,
                                                           const edm::EventSetup& iSetup,
                                                           trigger::TriggerFilterObjectWithRefs& filterproduct) const {
  // All HLT filters must create and fill an HLT filter object,
  // recording any reconstructed physics objects satisfying (or not)
  // this HLT filter, and place it in the Event.
  bool accept(false);

  std::vector<l1t::HPSPFTauRef> coll1;
  std::vector<l1t::HPSPFTauRef> coll2;

  if (getCollections(iEvent, coll1, coll2, filterproduct)) {
    int n(0);
    l1t::HPSPFTauRef r1;
    l1t::HPSPFTauRef r2;

    for (unsigned int i1 = 0; i1 != coll1.size(); i1++) {
      r1 = coll1[i1];
      unsigned int I(0);
      if (same_) {
        I = i1 + 1;
      }
      for (unsigned int i2 = I; i2 != coll2.size(); i2++) {
        r2 = coll2[i2];

        if (!computeDZ(iEvent, r1, r2))
          continue;

        n++;
        filterproduct.addObject(triggerType1_, r1);
        filterproduct.addObject(triggerType2_, r2);
      }
    }

    accept = accept || (n >= min_N_);
  }

  return accept;
}

typedef HLTDoubletDZ<reco::Electron, reco::Electron> HLT2ElectronElectronDZ;
typedef HLTDoubletDZ<reco::RecoChargedCandidate, reco::RecoChargedCandidate> HLT2MuonMuonDZ;
typedef HLTDoubletDZ<reco::Electron, reco::RecoChargedCandidate> HLT2ElectronMuonDZ;
typedef HLTDoubletDZ<reco::RecoEcalCandidate, reco::RecoEcalCandidate> HLT2PhotonPhotonDZ;
typedef HLTDoubletDZ<reco::RecoChargedCandidate, reco::RecoEcalCandidate> HLT2MuonPhotonDZ;
typedef HLTDoubletDZ<reco::RecoEcalCandidate, reco::RecoChargedCandidate> HLT2PhotonMuonDZ;
typedef HLTDoubletDZ<l1t::TrackerMuon, l1t::TrackerMuon> HLT2L1TkMuonL1TkMuonDZ;
typedef HLTDoubletDZ<l1t::PFTau, l1t::PFTau> HLT2L1PFTauL1PFTauDZ;
typedef HLTDoubletDZ<l1t::HPSPFTau, l1t::HPSPFTau> HLT2L1HPSPFTauL1HPSPFTauDZ;

DEFINE_FWK_MODULE(HLT2ElectronElectronDZ);
DEFINE_FWK_MODULE(HLT2MuonMuonDZ);
DEFINE_FWK_MODULE(HLT2ElectronMuonDZ);
DEFINE_FWK_MODULE(HLT2PhotonPhotonDZ);
DEFINE_FWK_MODULE(HLT2PhotonMuonDZ);
DEFINE_FWK_MODULE(HLT2MuonPhotonDZ);
DEFINE_FWK_MODULE(HLT2L1TkMuonL1TkMuonDZ);
DEFINE_FWK_MODULE(HLT2L1PFTauL1PFTauDZ);
DEFINE_FWK_MODULE(HLT2L1HPSPFTauL1HPSPFTauDZ);
