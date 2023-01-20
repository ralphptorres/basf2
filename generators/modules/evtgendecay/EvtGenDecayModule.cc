/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* Own header. */
#include <generators/modules/evtgendecay/EvtGenDecayModule.h>

/* Generators headers. */
#include <generators/evtgen/EvtGenUtilities.h>

/* Belle 2 headers. */
#include <framework/datastore/StoreArray.h>
#include <framework/utilities/FileSystem.h>

/* External headers. */
#include <EvtGenBase/EvtPDL.hh>
#include <EvtGenBase/EvtDecayTable.hh>

using namespace Belle2;

REG_MODULE(EvtGenDecay);

EvtGenDecayModule::EvtGenDecayModule() : Module()
{
  setDescription("This module decays unstable particles using EvtGen. The "
                 "event should be already generated by another generator. "
                 "If you need to generate full event with EvtGen, then use "
                 "the module 'EvtGenInput'.");
  addParam("DecFile", m_DecFile, "EvtGen decay file (DECAY.DEC).",
           FileSystem::findFile(
             "decfiles/dec/DECAY_BELLE2.DEC", true));
  addParam("UserDecFile", m_UserDecFile, "User EvtGen decay file.",
           std::string(""));
  addParam("MCParticleColName", m_MCParticleColName,
           "MCParticle collection name.", std::string(""));
  m_Initialized = false;
}

EvtGenDecayModule::~EvtGenDecayModule()
{
}

void EvtGenDecayModule::initialize()
{
  StoreArray<MCParticle> mcParticles(m_MCParticleColName);
  mcParticles.isRequired();
  generators::checkEvtGenDecayFile(m_DecFile);
}

void EvtGenDecayModule::beginRun()
{
}

void EvtGenDecayModule::event()
{
  int i, n;
  if (m_BeamParameters.hasChanged()) {
    if (!m_Initialized) {
      initializeGenerator();
    } else {
      B2FATAL("EvtGenDecayModule::event(): BeamParameters have changed within "
              "a job, this is not supported for EvtGen!");
    }
  }
  m_Graph.clear();
  m_Graph.loadList(m_MCParticleColName);
  n = m_Graph.size();
  for (i = 0; i < n; i++) {
    bool decay = true;
    MCParticleGraph::GraphParticle* graphParticle = &m_Graph[i];

    if (graphParticle->isInitial())
      decay = false;
    else if (graphParticle->getNDaughters() > 0)
      decay = false;
    else if (m_DecayableParticles.find(graphParticle->getPDG()) ==
             m_DecayableParticles.end())
      decay = false;
    if (decay)
      m_EvtGenInterface.simulateDecay(m_Graph, *graphParticle);
  }
  m_Graph.generateList(
    m_MCParticleColName,
    MCParticleGraph::c_clearParticles | MCParticleGraph::c_setDecayInfo |
    MCParticleGraph::c_checkCyclic);
}

void EvtGenDecayModule::endRun()
{
}

void EvtGenDecayModule::terminate()
{
}

void EvtGenDecayModule::initializeGenerator()
{
  int i, n;
  EvtId id;
  EvtDecayTable* decayTable;
  m_EvtGenInterface.setup(m_DecFile, "", m_UserDecFile);
  decayTable = EvtDecayTable::getInstance();
  n = EvtPDL::entries();
  for (i = 0; i < n; i++) {
    id = EvtPDL::getEntry(i);
    if (decayTable->getNModes(id) > 0)
      m_DecayableParticles.insert(EvtPDL::getStdHep(id));
  }
}

