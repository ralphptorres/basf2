/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
//Object with performing the actual algorithm:
#include <tracking/kinkFinding/fitter/kinkFitter.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/Track.h>

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

#include <TVector3.h>

#include <string>
#include <memory>

namespace Belle2 {

  /**
   * Kink finder module.
   *
   * Pairs up tracks,
   * tries to find vertices between them.
   *
   * The resulting pairs of tracks are stored as Belle2::Kink.
   */
  class KinkFinderModule : public Module {

  public:

    /** Setting of module description, parameters. */
    KinkFinderModule();

    /** Acknowledgement of destructor. */
    ~KinkFinderModule() override = default;

    /** Registration of StoreArrays, Relations. */
    void initialize() override;

    /** Creates Belle2::Kink from Belle2::Track as described in the class documentation. */
    void event() override;

    /** Prints status summary. */
    void terminate() override;

  private:

    std::string m_arrayNameTrack;  ///< StoreArray name of the Belle2::Track (Input).
    StoreArray <Track> m_tracks;  ///< StoreArray of Belle2::Track.

    std::unique_ptr<kinkFitter> m_kinkFitter;  ///< Object containing the algorithm of Kink creation.
    std::string m_arrayNameRecoTrack;  ///< StoreArray name of the RecoTrack (Input).
    std::string m_arrayNameCopiedRecoTrack;  ///< StoreArray name of the RecoTrack used for creating copies.
    std::string m_arrayNameTFResult;  ///< StoreArray name of the TrackFitResult (In- and Output).
    std::string m_arrayNameKink;  ///< StoreArray name of the Kink (Output).

    double m_vertexChi2Cut;  ///< Cut on Chi2 for the Kink vertex.
    double m_vertexDistanceCut;  ///< Cut on distance between tracks at the Kink vertex.
    int    m_kinkFitterMode;  ///< Fitter mode.
    double m_precutRho;  ///< Preselection cut on transverse shift from the outer CDC wall for the track ending points.
    double m_precutZ;  ///< Preselection cut on z shift from the outer CDC wall for the track ending points.
    double m_precutDistance;  ///< Preselection cut on distance between ending points of two tracks.
    double m_precutDistance2D;  ///< Preselection cut on 2D distance between ending points of two tracks (for bad z cases).

    /**
     * Test if the point in space is inside CDC (approximate custom geometry) with respect to shifts from outer wall.
     * @param pos point in space (TVector3 for MeasuredStateOnPlane)
     * @param shiftR transverse shift from the outer CDC wall
     * @param shiftZ z shift from the outer CDC wall
     * @return
     */
    bool ifInCDC(TVector3& pos, double shiftR, double shiftZ);

    /**
     * Check if the track can be a mother candidate based on some simple selections.
     * @param recoTrack track of the candidate
     * @return
     */
    bool preFilterMotherTracks(RecoTrack const* const recoTrack);

    /**
     * Check if the track can be a daughter candidate based on some simple selections.
     * @param recoTrack track of the candidate
     * @return
     */
    bool preFilterDaughterTracks(RecoTrack const* const recoTrack);

    /**
     * Track pair preselection based on distance between two tracks with different options.
     * Filter 1: Distance between first point of the daughter and last point of the mother < m_precutDistance (majority).
     * Filter 2: Distance between last point of the daughter and last point of the mother < m_precutDistance
     * (wrong daughter sign; minority but can be increased in future).
     * Filter 3: Distance between the daughter Helix extrapolation to last point of the mother
     * and last point of the mother < m_precutDistance (lost layers for daughter, second largest contribution).
     * Filter 4: 2D distance between first point of the daughter and last point of the mother < m_precutDistance2D
     * if the mother has less than 10 CDC hits (bad daughter resolution recovered by hit reassignment).
     * @param motherTrack mother track
     * @param daughterTrack daughter track
     * @return flag of the satisfied preselection criteria, or 0 otherwise.
     */
    short isTrackPairSelected(const Track* motherTrack, const Track* daughterTrack);

    /**
     * Kink fitting and storing
     * @param trackMother mother track
     * @param trackDaughter daughter track
     * @param filterFlag flag of the satisfied preselection criteria
     */
    void fitAndStore(const Track* trackMother, const Track* trackDaughter, short filterFlag);

    // counter for KinkFinder statistics
    int m_allStored = 0;    ///< counter for all saved Kinks
    int m_f1Stored = 0;    ///< counter for filter 1 saved Kinks
    int m_f2Stored = 0;    ///< counter for filter 2 saved Kinks
    int m_f3Stored = 0;    ///< counter for filter 3 saved Kinks
    int m_f4Stored = 0;    ///< counter for filter 4 saved Kinks
  };
}
