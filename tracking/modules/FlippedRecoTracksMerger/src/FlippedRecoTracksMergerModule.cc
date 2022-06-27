/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/modules/FlippedRecoTracksMerger/FlippedRecoTracksMergerModule.h>

using namespace Belle2;

REG_MODULE(FlippedRecoTracksMerger);

FlippedRecoTracksMergerModule::FlippedRecoTracksMergerModule() :
  Module()
{
  setDescription("Copies RecoTracks without their fit information.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("inputStoreArrayName", m_inputStoreArrayName,
           "Name of the input StoreArray");
  addParam("inputStoreArrayNameFlipped", m_inputStoreArrayNameFlipped,
           "Name of the input StoreArray for flipped tracks");
}

void FlippedRecoTracksMergerModule::initialize()
{
  //m_inputRecoTracks.isRequired(m_inputStoreArrayName);
}

void FlippedRecoTracksMergerModule::event()
{

  Belle2::StoreArray<RecoTrack> m_inputRecoTracks(m_inputStoreArrayName);
  Belle2::StoreArray<RecoTrack> m_inputRecoTracksFlipped(m_inputStoreArrayNameFlipped);
  Belle2::StoreArray<TrackFitResult> TrackFitResultsArray("TrackFitResults");

  // loop all the recoTracks
  for (RecoTrack& recoTrack : m_inputRecoTracks) {

    // check if the recoTracks was fitted successfully
    if (not recoTrack.wasFitSuccessful()) {
      continue;
    }
    // get the related Belle2::Tracks
    Track* b2track = recoTrack.getRelatedFrom<Belle2::Track>();

    if (b2track) {

      // get the cut from DB
      if (m_flipCutsFromDB.isValid()) {
        m_2nd_mva_cut = (*m_flipCutsFromDB).getSecondCut();

        // if both the 1st MVA and 2nd MVA were passed.
        if (!isnan(recoTrack.get2ndFlipQualityIndicator()) and (recoTrack.get2ndFlipQualityIndicator() > m_2nd_mva_cut)) {

          // get the related RecoTrack_flipped
          RecoTrack* RecoTrack_flipped =  recoTrack.getRelatedFrom<Belle2::RecoTrack>("RecoTracks_flipped");

          if (RecoTrack_flipped) {

            // get the Tracks_flipped
            Track* b2trackFlipped = RecoTrack_flipped->getRelatedFrom<Belle2::Track>("Tracks_flipped");
            if (b2trackFlipped) {
              std::vector<Track::ChargedStableTrackFitResultPair> fitResultsAfter = b2trackFlipped->getTrackFitResults("TrackFitResults_flipped");
              std::vector<Track::ChargedStableTrackFitResultPair> fitResultsBefore = b2track->getTrackFitResults();

              // loop over the original fitResults
              for (long unsigned int index = 0; index < fitResultsBefore.size() ; index++) {
                // update the fitResults
                if (index < fitResultsAfter.size()) {
                  auto fitResultAfter  = fitResultsAfter[index].second;

                  fitResultsBefore[index].second->updateTrackFitResult(*fitResultAfter);
                } else {
                  fitResultsBefore[index].second->maskThisFitResult();
                }
              }


              const auto& measuredStateOnPlane = recoTrack.getMeasuredStateOnPlaneFromLastHit();

              TVector3 currentPosition = measuredStateOnPlane.getPos();
              TVector3 currentMomentum = measuredStateOnPlane.getMom();
              double currentCharge = measuredStateOnPlane.getCharge();

              // revert the charge and momentum
              recoTrack.setChargeSeedOnly(-currentCharge);
              recoTrack.setPositionAndMomentumOnly(currentPosition,  -currentMomentum);

              // Reverse the SortingParameters
              auto RecoHitInfos = recoTrack.getRecoHitInformations();
              for (auto RecoHitInfo : RecoHitInfos) {
                RecoHitInfo->setSortingParameter(std::numeric_limits<unsigned int>::max() - RecoHitInfo->getSortingParameter());
              }

            }
          }
        }
      }
    }
  }
}
