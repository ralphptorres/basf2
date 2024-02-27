/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <iostream>
#include <fstream>
#include <sstream>

#include <klm/modules/KLMNNmuid/KLMNNmuidModule.h>

#include <klm/dataobjects/KLMHit2d.h>
#include <klm/dataobjects/KLMDigit.h>
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>
#include <klm/muid/MuidElementNumbers.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Const.h>

#include <tracking/dataobjects/ExtHit.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/variables/Variables.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mva/interface/Interface.h>

#include <boost/algorithm/string/predicate.hpp>

using namespace Belle2;

REG_MODULE(KLMNNmuid);

KLMNNmuidModule::KLMNNmuidModule() : Module()
{
  // Set module properties
  setDescription(R"DOC(Get information from KLMMuIDLikelihood)DOC");

  // Parameter definitions
  addParam("inputListName", m_inputListName,
           "list of input ParticleList name", std::string(""));
  addParam("hitChiCut", m_hitChiCut,
           "Hit chi cut", 3.5);
  addParam("identifier", m_identifier,
           "Database identifier or file used to load the weights.",
           m_identifier);
}

KLMNNmuidModule::~KLMNNmuidModule()
{
}

void KLMNNmuidModule::initialize()
{
  StoreObjPtr<ParticleList>().isRequired(m_inputListName);
  m_klmHit2ds.isRequired();

  if (not(boost::ends_with(m_identifier, ".root") or boost::ends_with(m_identifier, ".xml"))) {
    m_weightfile_representation = std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>(new
                                  DBObjPtr<DatabaseRepresentationOfWeightfile>(m_identifier));
  }
  MVA::AbstractInterface::initSupportedInterfaces();
}

void KLMNNmuidModule::terminate()
{
  m_expert.reset();
  m_dataset.reset();
}

void KLMNNmuidModule::beginRun()
{
  if (m_weightfile_representation) {
    if (m_weightfile_representation->hasChanged()) {
      std::stringstream ss((*m_weightfile_representation)->m_data);
      auto weightfile = MVA::Weightfile::loadFromStream(ss);
      init_mva(weightfile);
    }
  } else {
    auto weightfile = MVA::Weightfile::loadFromFile(m_identifier);
    init_mva(weightfile);
  }
}

void KLMNNmuidModule::endRun()
{
}

void KLMNNmuidModule::init_mva(MVA::Weightfile& weightfile)
{
  auto supported_interfaces = MVA::AbstractInterface::getSupportedInterfaces();
  MVA::GeneralOptions general_options;
  weightfile.getOptions(general_options);
  m_expert = supported_interfaces[general_options.m_method]->getExpert();
  m_expert->load(weightfile);
  std::vector<float> dummy;
  /* The number of input variables depends on the experiment. */
  int nInputVariables = 121;
  dummy.resize(nInputVariables, 0);
  m_dataset = std::unique_ptr<MVA::SingleDataset>(new MVA::SingleDataset(general_options, std::move(dummy), 0));
}


void KLMNNmuidModule::event()
{

  const StoreObjPtr<ParticleList> plist(m_inputListName);

  const unsigned int nParticles = plist->getListSize();

  for (unsigned int iPart = 0; iPart < nParticles; iPart++) {

    Particle* part = plist->getParticle(iPart);

    auto track = part->getTrack();

    KLMMuidLikelihood* klmll = track->getRelatedTo<KLMMuidLikelihood>();

    if (!klmll) continue;

    // initialize hit pattern arrays
    for (int layer = 0; layer < 29; layer++) {
      m_hitpattern_steplength[layer] = -1;
      m_hitpattern_width[layer] = -1;
      m_hitpattern_chi2[layer] = -1;
      m_hitpattern_hasext[layer] = 0;
    }

    std::map<int, ExtHit*> ExtHitMap; // study if using klmll ext pattern would have equivalent performance.
    for (ExtHit& exthit : track->getRelationsTo<ExtHit>()) {

      if (exthit.getDetectorID() < 0x100) continue; // BKLM = 0x107, EKLM = 0x207

      int layer;
      int inBKLM = (exthit.getDetectorID() == Const::EDetector::BKLM);
      int copyid = exthit.getCopyID();

      int section, sector, plane, strip;

      if (inBKLM) {
        BKLMElementNumbers::moduleNumberToElementNumbers(copyid, &section, &sector, &layer);
        m_hitpattern_hasext[layer - 1] = 1;
      } else {
        EKLMElementNumbers::Instance().stripNumberToElementNumbers(copyid, &section, &layer, &sector, &plane, &strip);
        m_hitpattern_hasext[15 + layer - 1] = 1;
      }

      int index = (1 - inBKLM) * 100 + layer; // make sure BKLM hits is in front of EKLM hits
      ExtHitMap[index] = &exthit; // only keep the last ext hit in each layer
    }
    int nklmexthits = ExtHitMap.size();

    std::multimap<int, KLMHit2d*> Hit2dMap;
    for (KLMHit2d& klmhit : track->getRelationsTo<KLMHit2d>("", "chidimx")) {
      unsigned long int hit_inBKLM = (klmhit.getSubdetector() == KLMElementNumbers::c_BKLM);
      unsigned long int hit_layer = klmhit.getLayer();
      unsigned long int hit_sector = klmhit.getSector();
      unsigned long int hit_section = klmhit.getSection();

      int index = (1 - hit_inBKLM) * 100 + hit_layer; // BKLM hits are in front of EKLM hits
      Hit2dMap.insert(std::pair<int, KLMHit2d*> {index,  &klmhit}); // multimap is used here so all hits are kept even if they have the same index
    }

    int nklmhits = 0;
    //int nklmclusters = 0;
    float previoushitposition[3] = {0.};
    RelationVector<KLMHit2d> KLMHit2drelationx = track->getRelationsTo<KLMHit2d>("", "chidimx");
    RelationVector<KLMHit2d> KLMHit2drelationy = track->getRelationsTo<KLMHit2d>("", "chidimy");
    RelationVector<KLMHit2d> KLMHit2drelationchi2 = track->getRelationsTo<KLMHit2d>("", "intersectchisq");
    int KFndof = 0;
    for (auto itermap = Hit2dMap.begin(); itermap != Hit2dMap.end(); itermap ++) {

      KLMHit2d* klmhitadd = itermap->second;
      KLMHit2d klmhit = *klmhitadd;
      int index = itermap->first;

      nklmhits += 1;
      double  hit_Xposition = klmhit.getPositionX();
      double  hit_Yposition = klmhit.getPositionY();
      double  hit_Zposition = klmhit.getPositionZ();
      double  hit_MinStripXposition = klmhit.getPositionXOfMinStrip();
      double  hit_MinStripYposition = klmhit.getPositionYOfMinStrip();
      double  hit_MinStripZposition = klmhit.getPositionZOfMinStrip();
      unsigned long int hit_Phistripmin;
      unsigned long int hit_Phistripmax;
      unsigned long int hit_Zstripmin  ;
      unsigned long int hit_Zstripmax  ;

      unsigned long int hit_inBKLM = (klmhit.getSubdetector() == KLMElementNumbers::c_BKLM);
      if (hit_inBKLM) {
        hit_Phistripmin = klmhit.getPhiStripMin();
        hit_Phistripmax = klmhit.getPhiStripMax();
        hit_Zstripmin   = klmhit.getZStripMin();
        hit_Zstripmax   = klmhit.getZStripMax();
      } else {
        hit_Phistripmin = klmhit.getXStripMin();
        hit_Phistripmax = klmhit.getXStripMax();
        hit_Zstripmin = klmhit.getYStripMin();
        hit_Zstripmax = klmhit.getYStripMax();
      }
      unsigned long int hit_layer = klmhit.getLayer();
      //unsigned long int hit_sector = klmhit.getSector();
      //unsigned long int hit_section = klmhit.getSection();

      // find the hit index in relations
      int targethit = -1;
      float KFchix = 999.;
      float KFchiy = 999.;
      float KFchi2 = 999.;
      for (unsigned int iHit = 0; iHit < KLMHit2drelationx.size(); iHit++) {
        KLMHit2d* klmhit1 = KLMHit2drelationx[iHit];
        if (klmhitadd == klmhit1) {
          targethit = iHit;
          break;
        }
      }
      if (targethit > -1) {
        KFchix = KLMHit2drelationx.weight(targethit);
        KFchiy = KLMHit2drelationy.weight(targethit);
        KFchi2 = KLMHit2drelationchi2.weight(targethit);
        part->writeExtraInfo("Hitchix_" + std::to_string(nklmhits - 1), KFchix);
        part->writeExtraInfo("Hitchiy_" + std::to_string(nklmhits - 1), KFchiy);
        part->writeExtraInfo("Hitchi2_" + std::to_string(nklmhits - 1), KFchi2);
      }

      float width;
      if (hit_inBKLM) {
        float phiwidth = sqrt(pow(hit_Xposition - hit_MinStripXposition, 2) + pow(hit_Yposition - hit_MinStripYposition, 2)) + 2;
        width = sqrt(pow(phiwidth, 2) + pow(fabs(hit_Zposition - hit_MinStripZposition) + 2, 2));
      } else {
        width = sqrt(pow(fabs(hit_Xposition - hit_MinStripXposition) + 2, 2) + pow(fabs(hit_Yposition - hit_MinStripYposition) + 2, 2));
      }
      part->writeExtraInfo("Hitwidth_" + std::to_string(nklmhits - 1), width);

      int Hit_selected = (int)(KFchix < m_hitChiCut && KFchiy < m_hitChiCut);
      part->writeExtraInfo("Hitselected_" + std::to_string(nklmhits - 1), Hit_selected);

      if (not Hit_selected) continue;
      KFndof += 2;
      // penetration depth calculation. only the first hit on each are used
      float steplength = 0.;
      if (KFndof >
          2) { // replace this with nklmhits after adjustment in extrapolation module. Since we don't calculate chi2 in this module any more.
        steplength = sqrt(pow((hit_Xposition - previoushitposition[0]), 2) + pow((hit_Yposition - previoushitposition[1]),
                          2) + pow((hit_Zposition - previoushitposition[2]), 2));
      }
      previoushitposition[0] = hit_Xposition;
      previoushitposition[1] = hit_Yposition;
      previoushitposition[2] = hit_Zposition;

      // hit pattern creation. All selected hits will be used
      int hitpatternindex = hit_layer - 1 + 15 * (1 - hit_inBKLM);
      m_hitpattern_chi2[hitpatternindex] = KFchi2;
      m_hitpattern_steplength[hitpatternindex] = steplength;
      m_hitpattern_width[hitpatternindex] = width;
    } // loop of Hit2dMap

    part->writeExtraInfo("nklmhits", nklmhits);
    part->writeExtraInfo("KFndof", KFndof);

    ExtHitMap.clear();
    Hit2dMap.clear();

    double muprob_nn = getNNmuProbability(part, klmll);
    part->writeExtraInfo("muprob_nn", muprob_nn);
  } // loop of particles
}

float KLMNNmuidModule::getNNmuProbability(const Particle* part, const KLMMuidLikelihood* klmll)
{
  m_dataset->m_input[0] = klmll->getChiSquared();
  m_dataset->m_input[1] = klmll->getDegreesOfFreedom();
  m_dataset->m_input[2] = klmll->getExtLayer() - klmll->getHitLayer();
  m_dataset->m_input[3] = klmll->getExtLayer();
  m_dataset->m_input[4] = Variable::particlePt(part);

  int MAXLAYER = 29;

  for (int layer = 0; layer < MAXLAYER; layer ++) {
    m_dataset->m_input[5 + 4 * layer + 0] = m_hitpattern_width[layer]; // width
    m_dataset->m_input[5 + 4 * layer + 1] = m_hitpattern_steplength[layer]; // steplength
    m_dataset->m_input[5 + 4 * layer + 2] = m_hitpattern_chi2[layer]; // chi2
    m_dataset->m_input[5 + 4 * layer + 3] = m_hitpattern_hasext[layer]; // hasext
  }

  float muprob_nn = m_expert->apply(*m_dataset)[0];
  return muprob_nn;
}


