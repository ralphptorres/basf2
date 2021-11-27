/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/trackFilter/ParallelTrackFilterModule.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/HitPatternVXD.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <framework/datastore/StoreArray.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ParallelTrackFilter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ParallelTrackFilterModule::ParallelTrackFilterModule() : Module()
{
  // Set module properties
  setDescription("Generates a new StoreArray from the input StoreArray which contains only tracks that meet the specified criteria.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  addParam("inputArrayName", m_inputArrayName, "StoreArray with the input tracks", std::string("Tracks"));
  addParam("outputINArrayName", m_outputINArrayName, "StoreArray with the output tracks", std::string("TracksIN"));
  addParam("outputOUTArrayName", m_outputOUTArrayName, "StoreArray with the output tracks", std::string("TracksOUT"));

  //selection parameter definition
  addParam("min_d0", m_min_d0, "minimum value of the d0", double(-100));
  addParam("max_d0", m_max_d0, "maximum value of the d0", double(+100));
  addParam("min_z0", m_min_z0, "minimum value of the z0", double(-500));
  addParam("max_z0", m_max_z0, "maximum value of the z0", double(+500));
  addParam("min_pCM", m_min_pCM, "minimum value of the center-of-mass-momentum", double(0));
  addParam("min_pT", m_min_pT, "minimum value of the transverse momentum", double(0));
  addParam("min_Pvalue", m_min_Pval, "minimum value of the P-Value of the track fit", double(0));
  addParam("min_NumHitPXD", m_min_NumHitsPXD, "minimum number of PXD hits associated to the trcak", int(0));
  addParam("min_NumHitSVD", m_min_NumHitsSVD, "minimum number of SVD hits associated to the trcak", int(0));
  addParam("min_NumHitCDC", m_min_NumHitsCDC, "minimum number of CDC hits associated to the trcak", int(0));

}


void ParallelTrackFilterModule::initialize()
{
  B2DEBUG(22, "ParallelTrackFilterModule " + getName() + " parameters:"
          << LogVar("inputArrayName", m_inputArrayName)
          << LogVar("outputINArrayName", m_outputINArrayName)
          << LogVar("outputOUTArrayName", m_outputOUTArrayName));

  // Attempt the module initialization
  initializeSelectSubset();
}


void ParallelTrackFilterModule::beginRun()
{
  // If the initialization was not done because the input StoreArray was missing, retry
  // Otherwise, this call does nothing
  initializeSelectSubset();
}


void ParallelTrackFilterModule::initializeSelectSubset()
{

  // Attepmt SelectSubset<Track> initialization only if not done already
  if (m_selectedTracks.getSet())
    return;

  // Can't initialize if the input array is not present (may change from run to run)
  StoreArray<Track> inputArray(m_inputArrayName);
  if (!inputArray.isOptional()) {
    B2WARNING("Missing input tracks array, " + getName() + " is skipped for this run"
              << LogVar("inputArrayName", m_inputArrayName));
    return;
  }

  m_selectedTracks.registerSubset(inputArray, m_outputINArrayName);
  m_selectedTracks.inheritAllRelations();

  m_notSelectedTracks.registerSubset(inputArray, m_outputOUTArrayName);
  m_notSelectedTracks.inheritAllRelations();

}


void ParallelTrackFilterModule::event()
{

  StoreArray<Track> inputArray(m_inputArrayName);
  if (!inputArray.isOptional() || !m_selectedTracks.getSet()) {
    B2DEBUG(22, "Missing Tracks array, " + getName() + " is skipped." << LogVar("inputArrayName", m_inputArrayName));
    return;
  }

  m_selectedTracks.select([this](const Track * track) {
    return this->isSelected(track);
  });

  m_notSelectedTracks.select([this](const Track * track) {
    return !this->isSelected(track);
  });

}

bool ParallelTrackFilterModule::isSelected(const Track* track)
{
  const TrackFitResult* tfr = track->getTrackFitResult(Const::ChargedStable(Const::pion.getPDGCode()));
  if (tfr == nullptr)
    return false;

  if (tfr->getD0() < m_min_d0 || tfr->getD0() > m_max_d0)
    return false;

  if (tfr->getZ0() < m_min_z0 || tfr->getZ0() > m_max_z0)
    return false;

  if (tfr->getPValue() < m_min_Pval)
    return false;

  if (tfr->getMomentum().Perp() < m_min_pT)
    return false;

  HitPatternVXD hitPatternVXD = tfr->getHitPatternVXD();
  if (hitPatternVXD.getNSVDHits() < m_min_NumHitsSVD ||  hitPatternVXD.getNPXDHits() < m_min_NumHitsPXD)
    return false;

  HitPatternCDC hitPatternCDC = tfr->getHitPatternCDC();
  if (hitPatternCDC.getNHits() < m_min_NumHitsCDC)
    return false;

  return true;
}
