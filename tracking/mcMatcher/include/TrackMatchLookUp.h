/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {

  /*!
   * Class to provide convenient methods to look up matching information between pattern
   * recognition and Monte Carlo tracks.
   *
   * It provides a more user friendly look up interface into the relations that were
   * generated by the MCTrackMatcherModule.
   *
   * Instead of looking through the relations directly to find the matching information on your own
   * the methods of this class exhibit a conceptual view on them.
   *
   * This class is also accessible in python steering files.
   */
  class TrackMatchLookUp {

  public:
    /*!
     * Matching categories for the Monte Carlo tracks.
     * c_undefined, //until the matcher module sets it
     * c_matched, // hit pattern and charge are both correct
     * c_matchedWrongCharge, // hit pattern is correct, but the charge is wrong
     * c_merged, //a clone with the correct charge
     * c_mergedWrongCharge, //a clone with the wrong charge
     * c_missing
     */
    enum class MCToPRMatchInfo { c_undefined, c_matched, c_matchedWrongCharge, c_merged, c_mergedWrongCharge, c_missing };

    /*!
     * Matching categories for the pattern recognition tracks.
     * c_undefined, //until the matcher module sets it
     * c_matched, // hit pattern and charge are both correct
     * c_matchedWrongCharge, // hit pattern is correct, but the charge is wrong
     * c_clone, //a clone with the correct charge
     * c_cloneWrongCharge, //a clone with the wrong charge
     * c_background,
     * c_ghost
     */
    enum class PRToMCMatchInfo { c_undefined, c_matched, c_matchedWrongCharge, c_clone, c_cloneWrongCharge, c_background, c_ghost };

  public:
    /*!
     * Constructor taking the names of the StoreArrays containing the Monte Carlo tracks
     * and the pattern recognition tracks respectivelly.
     *
     * Only the name of StoreArray of the Monte Carlo reference tracks has to be given.
     * The name of the StoreArray of the pattern recognition tracks to defaults to the standard
     * name.
     */
    explicit TrackMatchLookUp(const std::string& mcRecoTrackStoreArrayName,
                              const std::string& prRecoTrackStoreArrayName = "");

  private:
    /*!
     * Helper function to assume the correct matching category for the Monte Carlo tracks from the
     * information efficiency relation.
     */
    MCToPRMatchInfo extractMCToPRMatchInfo(const RecoTrack& mcRecoTrack,
                                           const RecoTrack* prRecoTrack,
                                           const float& efficiency) const;

    /*!
     * Helper function to assume the correct matching category for the pattern recognition tracks
     * from the information purity relation.
     */
    PRToMCMatchInfo extractPRToMCMatchInfo(const RecoTrack& prRecoTrack,
                                           const RecoTrack* mcRecoTrack,
                                           const float& purity) const;

    /*!
     * Looks up the matched Monte Carlo track with the PRToMCMatchInfo matchingStatus
     * for the given pattern recognition track.
     * If there is no match or the pattern recognition track is a clone return nullptr.
     */
    const RecoTrack* getMCRecoTrackWithStatus(const RecoTrack& prRecoTrack,
                                              const PRToMCMatchInfo matchingStatus = PRToMCMatchInfo::c_matched) const;

    /*!
     * Looks up the matched pattern recognition track with the MCToPRMatchInfo matchingStatus
     * for the given Monte Carlo track.
     * If there is no match or the Monte Carlo track return nullptr.
     */
    const RecoTrack* getPRRecoTrackWithStatus(const RecoTrack& mcRecoTrack,
                                              const MCToPRMatchInfo matchingStatus = MCToPRMatchInfo::c_matched) const;

  public:
    /*!
     * Looks up the matched Monte Carlo track for the given pattern recognition track.
     * If there is no match or the pattern recognition track is a clone, or
     * the charge is wrong return nullptr.
     */
    const RecoTrack* getCorrectChargeMatchedMCRecoTrack(const RecoTrack& prRecoTrack) const
    {
      return getMCRecoTrackWithStatus(prRecoTrack, PRToMCMatchInfo::c_matched);
    }

    /*!
      * Looks up the matched Monte Carlo track for the given pattern recognition track
      * with the wrong charge.
      * If there is no match or the pattern recognition track is a clone, or the charge is
      * correct return nullptr.
      */
    const RecoTrack* getWrongChargeMatchedMCRecoTrack(const RecoTrack& prRecoTrack) const
    {
      return getMCRecoTrackWithStatus(prRecoTrack, PRToMCMatchInfo::c_matchedWrongCharge);
    }

    /*!
     * Check whether any matched MC RecoTracks for the given prRecoTrack, independent of whether
     * both pattern and charge are matched correctly, or only the pattern is correct but the charge is wrong.
     * If no match exists at all, a nullptr is returned.
     */
    const RecoTrack* getAnyChargeMatchedMCRecoTrack(const RecoTrack& prRecoTrack) const
    {
      const RecoTrack* anyMatchedRecoTrack = getMCRecoTrackWithStatus(prRecoTrack, PRToMCMatchInfo::c_matched);
      if (anyMatchedRecoTrack == nullptr) {
        anyMatchedRecoTrack = getMCRecoTrackWithStatus(prRecoTrack, PRToMCMatchInfo::c_matchedWrongCharge);
      }
      return anyMatchedRecoTrack;
    }

    /*!
     * Looks up the matched pattern recognition track for the given Monte Carlo track.
     * If there is no match or the Monte Carlo track is a over-merged, or the
     * charge is wrong return nullptr.
     */
    const RecoTrack* getCorrectChargeMatchedPRRecoTrack(const RecoTrack& mcRecoTrack) const
    {
      return getPRRecoTrackWithStatus(mcRecoTrack, MCToPRMatchInfo::c_matched);
    }

    /*!
      * Looks up the matched pattern recognition track for the given Monte Carlo track.
      * If there is no match or the Monte Carlo track is a over-merged or the
      * charge is correct return nullptr.
      */
    const RecoTrack* getWrongChargeMatchedPRRecoTrack(const RecoTrack& mcRecoTrack) const
    {
      return getPRRecoTrackWithStatus(mcRecoTrack, MCToPRMatchInfo::c_matchedWrongCharge);
    }

    /*!
     * Check whether any matched PR RecoTracks for the given mcRecoTrack, independent of whether
     * both pattern and charge are matched correctly, or only the pattern is correct but the charge is wrong.
     * If no match exists at all, a nullptr is returned.
     */
    const RecoTrack* getAnyChargeMatchedPRRecoTrack(const RecoTrack& mcRecoTrack) const
    {
      const RecoTrack* anyMatchedRecoTrack = getPRRecoTrackWithStatus(mcRecoTrack, MCToPRMatchInfo::c_matched);
      if (anyMatchedRecoTrack == nullptr) {
        anyMatchedRecoTrack = getPRRecoTrackWithStatus(mcRecoTrack, MCToPRMatchInfo::c_matchedWrongCharge);
      }
      return anyMatchedRecoTrack;
    }

  public:
    /*!
     * Get the hit purity of the matched track.
     *
     * The given track can be both Monte Carlo track or pattern recognition track since
     * the match is one to one relation.
     * The returned purity is positive, hence the absolute value of the relation weight.
     * If no exact match is associated with the track return NAN.
     */
    float getMatchedPurity(const RecoTrack& recoTrack) const;

    /*!
     * Get the hit efficiency of the matched track.
     *
     * The given track can be both Monte Carlo track or pattern recognition track
     * since the match is one to one relation.
     * The returned efficiency is positive, hence the absolute value of the relation weight.
     * If no exact match is assoziated with the track return NAN.
     */
    float getMatchedEfficiency(const RecoTrack& recoTrack) const;


    /*!
     * Checks if the recoTrack charge is correctly assigned.
     *
     * Returns NAN if the recoTrack is not matched.
     * The given track can be both Monte Carlo track or pattern recognition track.
     */
    bool isChargeMatched(const RecoTrack& recoTrack) const;

  public:
    /*!
     * Gets the matching category of Monte Carlo track.
     */
    MCToPRMatchInfo getMCToPRMatchInfo(const RecoTrack& mcRecoTrack) const
    {
      float efficiency = NAN;
      const RecoTrack* prRecoTrack = getRelatedPRRecoTrack(mcRecoTrack, efficiency);
      return extractMCToPRMatchInfo(mcRecoTrack, prRecoTrack, efficiency);
    }

    /*!
     * Checks, if the pattern recognition track was matched to a Monte Carlo track
     * with both hit pattern and charge being matched.
     */
    bool isCorrectChargeMatchedPRRecoTrack(const RecoTrack& prRecoTrack) const
    {
      return getPRToMCMatchInfo(prRecoTrack) == PRToMCMatchInfo::c_matched;
    }

    /*!
    * Checks, if the pattern recognition track was matched to a Monte Carlo track
    * but with the wrong charge.
    */
    bool isWrongChargeMatchedPRRecoTrack(const RecoTrack& prRecoTrack) const
    {
      return getPRToMCMatchInfo(prRecoTrack) == PRToMCMatchInfo::c_matchedWrongCharge;
    }

    /*!
    * Checks, if the pattern recognition track was matched to a Monte Carlo track,
    * independent of whether the charge is matched.
    */
    bool isAnyChargeMatchedPRRecoTrack(const RecoTrack& prRecoTrack) const
    {
      return getPRToMCMatchInfo(prRecoTrack) == PRToMCMatchInfo::c_matched or
             getPRToMCMatchInfo(prRecoTrack) == PRToMCMatchInfo::c_matchedWrongCharge;
    }

    /*!
     * Checks, if the pattern recognition track is a clone of an other pattern recognition track.
     */
    bool isCorrectChargeClonePRRecoTrack(const RecoTrack& prRecoTrack) const
    {
      return getPRToMCMatchInfo(prRecoTrack) == PRToMCMatchInfo::c_clone;
    }

    /*!
     * Checks, if the pattern recognition track is a clone of an other pattern recognition track,
     * but with the wrong charge
     */
    bool isWrongChargeClonePRRecoTrack(const RecoTrack& prRecoTrack) const
    {
      return getPRToMCMatchInfo(prRecoTrack) == PRToMCMatchInfo::c_cloneWrongCharge;
    }

    /*!
     * Checks, if the pattern recognition track is a clone of an other pattern recognition track,
     * independent of whether the charge is correct
     */
    bool isAnyChargeClonePRRecoTrack(const RecoTrack& prRecoTrack) const
    {
      return getPRToMCMatchInfo(prRecoTrack) == PRToMCMatchInfo::c_clone or
             getPRToMCMatchInfo(prRecoTrack) == PRToMCMatchInfo::c_cloneWrongCharge;
    }

    /*!
     * Checks, if the pattern recognition track is mostly made from background hits
     */
    bool isBackgroundPRRecoTrack(const RecoTrack& prRecoTrack) const
    {
      return getPRToMCMatchInfo(prRecoTrack) == PRToMCMatchInfo::c_background;
    }

    /*!
     * Checks, if the pattern recognition track has contributions of different Monte Carlo tracks
     * and/or background hits, such that a match is not possible.
     */
    bool isGhostPRRecoTrack(const RecoTrack& prRecoTrack) const
    {
      return getPRToMCMatchInfo(prRecoTrack) == PRToMCMatchInfo::c_ghost;
    }

  public:
    /*!
     * Gets the matching category of pattern recognition track.
     */
    PRToMCMatchInfo getPRToMCMatchInfo(const RecoTrack& prRecoTrack) const
    {
      float purity = NAN;
      const RecoTrack* mcRecoTrack = getRelatedMCRecoTrack(prRecoTrack, purity);
      return extractPRToMCMatchInfo(prRecoTrack, mcRecoTrack, purity);
    }

    /*!
     * Checks, if the Monte Carlo Track was matched to a pattern recognition track
     * based on the hit pattern and the correct charge.
     */
    bool isCorrectChargeMatchedMCRecoTrack(const RecoTrack& mcRecoTrack) const
    {
      return getMCToPRMatchInfo(mcRecoTrack) == MCToPRMatchInfo::c_matched;
    }

    /*!
     * Checks, if the Monte Carlo Track was matched to a pattern recognition track
     * based on the hit pattern, but was assigned the wrong charge.
     */
    bool isWrongChargeMatchedMCRecoTrack(const RecoTrack& mcRecoTrack) const
    {
      return getMCToPRMatchInfo(mcRecoTrack) == MCToPRMatchInfo::c_matchedWrongCharge;
    }

    /*!
     * Checks, if the Monte Carlo Track was matched to a pattern recognition track
     * independent of the charge.
     */
    bool isAnyChargeMatchedMCRecoTrack(const RecoTrack& mcRecoTrack) const
    {
      return getMCToPRMatchInfo(mcRecoTrack) == MCToPRMatchInfo::c_matched or
             getMCToPRMatchInfo(mcRecoTrack) == MCToPRMatchInfo::c_matchedWrongCharge;
    }

    /*!
     * Checks, if the Monte Carlo Track has been merged into another pattern recognition track.
     */
    bool isCorrectChargeMergedMCRecoTrack(const RecoTrack& mcRecoTrack) const
    {
      return getMCToPRMatchInfo(mcRecoTrack) == MCToPRMatchInfo::c_merged;
    }

    /*!
     * Checks, if the Monte Carlo Track has been merged into another pattern recognition track
     * and with the wrong charge.
     */
    bool isWrongChargeMergedMCRecoTrack(const RecoTrack& mcRecoTrack) const
    {
      return getMCToPRMatchInfo(mcRecoTrack) == MCToPRMatchInfo::c_mergedWrongCharge;
    }

    /*!
     * Checks, if the Monte Carlo Track has been merged into another pattern recognition track.
     */
    bool isAnyChargeMergedMCRecoTrack(const RecoTrack& mcRecoTrack) const
    {
      return getMCToPRMatchInfo(mcRecoTrack) == MCToPRMatchInfo::c_merged or
             getMCToPRMatchInfo(mcRecoTrack) == MCToPRMatchInfo::c_mergedWrongCharge;
    }

    /*!
     *  Checks, if the Monte Carlo Track has no corresponding pattern recognition track.
     */
    bool isMissingMCRecoTrack(const RecoTrack& mcRecoTrack) const
    {
      return getMCToPRMatchInfo(mcRecoTrack) == MCToPRMatchInfo::c_missing;
    }

  public:
    /*!
     * Looks for a relation of the given track to a Monte Carlo particle.
     *
     * Returns nullptr if not found.
     *
     * Works for both pattern recognition and Monte Carlo tracks.
     *
     * Whether clones have a related MCParticle depends on the settings of the MCTrackMatcher
     * module that filled the relations.
     */
    const MCParticle* getRelatedMCParticle(const RecoTrack& recoTrack) const;

    /*!
     * Looks up the TrackFitResult of a pattern recognition track.
     *
     * Return nullptr if there is no related track fit result.
     *
     * The relations are setup to be from the Belle2::Track to the RecoTrack.
     *
     * Then the TrackFitResults stored within the Belle2::Tracks are retrieved and
     * the one selected particle hypothesises given via chargedStable is returned.
     *
     * The default is the Pion fit.
     */
    const TrackFitResult*
    getRelatedTrackFitResult(const RecoTrack& prRecoTrack,
                             Const::ChargedStable chargedStable = Const::pion) const;

    /*!
     * Looks for a related Monte Carlo track for the given pattern recognition track and
     * return it if found. Return nullptr if not found.
     */
    const RecoTrack* getRelatedMCRecoTrack(const RecoTrack& prRecoTrack) const;

    /*!
     * Looks for a related pattern recognition track for the given Monte Carlo track and
     * return it if found. Return nullptr if not found.
     */
    const RecoTrack* getRelatedPRRecoTrack(const RecoTrack& mcRecoTrack) const;

  public:
    /*!
     * Getter for the absolute value of the purity that is stored in the purity relation from
     * pattern recognition tracks to Monte Carlo tracks.
     *
     * The purity relation on the DataStore encodes some information about the match category in
     * the sign of the purity.
     * However user code is always interested in the absolute value of the purity but may forget
     * apply an absolute value.
     * Since the matching category can be found by other methods of this class, we only provide the
     * absolute value here.
     */
    float getRelatedPurity(const RecoTrack& prRecoTrack) const;

    /*!
     * Getter for the absolute value of the efficiency that is stored in the efficiency relation
     * from Monte Carlo tracks to pattern recognition tracks.
     *
     * The efficiency relation on the DataStore encodes some information about the match category
     * in the sign of the efficiency.
     * However user code is always interested in the absolute value of the efficiency but may
     * forget apply an absolute value.
     * Since the matching category can be found by other methods of this class, we only provide the
     * absolute value here.
     */
    float getRelatedEfficiency(const RecoTrack& mcRecoTrack) const;

    /*!
     * Helper function looking for a related Monte Carlo track to the given pattern recognition
     * track in the purity relation. Also returns to found purity as output parameter.
     */
    const RecoTrack* getRelatedMCRecoTrack(const RecoTrack& prRecoTrack, float& purity) const;

    /*!
     * Helper function looking for a related Monte Carlo track to the given pattern recognition
     * track in the efficiency relation. Also returns to found efficiency as output parameter.
     */
    const RecoTrack* getRelatedPRRecoTrack(const RecoTrack& mcRecoTrack, float& efficiency) const;

  public:
    /*!
     * Checks if the given track is in the Monte Carlo track StoreArray.
     */
    bool isMCRecoTrack(const RecoTrack& recoTrack) const;

    /*!
     * Checks if the given track is in the pattern recognition StoreArray.
     */
    bool isPRRecoTrack(const RecoTrack& recoTrack) const;

  public:
    /*!
     * Getter for the name of the StoreArray of the Monte Carlo tracks.
     */
    const std::string& getMCTracksStoreArrayName() const
    {
      return m_mcTracksStoreArrayName;
    }

    /*!
     * Getter for the name of the StoreArray of the pattern recognition tracks.
     */
    const std::string& getPRTracksStoreArrayName() const
    {
      return m_prTracksStoreArrayName;
    }

  private:
    //! Name of the StoreArray of Monte Carlo tracks
    std::string m_mcTracksStoreArrayName;

    //! Name of the StoreArray of Pattern recognition tracks
    std::string m_prTracksStoreArrayName;
  };
}
