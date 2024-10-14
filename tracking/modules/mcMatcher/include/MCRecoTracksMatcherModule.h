/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/MCParticle.h>


namespace Belle2 {
  /*!
   *  This module compares tracks generated by some pattern recognition algorithm for PXD, SVD and CDC to ideal Monte Carlo tracks
   *  and performs a matching from the former to the underlying MCParticles.
   *
   *  To achieve this it evaluates and saves the purities and efficiencies from the pattern recognition track to Monte-Carlo tracks,
   *  which can than also be used by a subsequent evaluation modules.
   *
   *  In order to match the tracks the module takes two StoreArrays of RecoTracks, which should be compared.
   *
   *  One of them contains RecoTracks composed by the pattern recognition algorithm to be assessed. They are referred to as PRTracks.
   *
   *  The second StoreArray holds the reference tracks, which should ideally be reconstructed. These are referred to as MCTracks and
   *  should generally be composed by the MCTrackingModule.
   *  (Design note : We use the tracks composed by the MCTrackFinder as reference, because the mere definition of
   *   what a trackable particle and what the best achievable track is, lies within the implementation of the MCTrackFinder.
   *   If we did not use the tracks from the MCTrackFinder as input, it would mean a double implementation of great parts of that logic.)
   *
   *  If the pattern recognition only covers a part of the tracking detectors, the matching can be constrained
   *  to specific subdetectors by switching of the appropriate usePXDHits, useSVDHits or useCDCHits parameters.
   *
   *  As a result the module
   *   -# creates a RelationArray from the PRTracks to the MCTracks, which will be called the purity relation,
   *   -# creates a RelationArray from the MCTracks to the PRTracks, which will be called the efficiency relation,
   *   -# assigns the McTrackId property of the PRTracks and
   *   -# creates a RelationArray from the PRTracks to the MCParticles.
   *   .
   *
   *  The RelationArray for purity and efficiency generally store only the single highest purity and
   *  the single highest efficiency for a given PRTrack, MCTrack respectivelly. However these values are stored with
   *  a minus sign if the PRTrack is a clone, or the MCTrack is merged into another PRTrack. The McTrackId is either set to the
   *  index of the MCParticle or to some negative value indicating the severity of the mismatch. (Classification details below).
   *
   *  Moreover, only PRTracks that exceed the minimal purity requirement and a minimal efficiency requirement
   *  will have their purity/efficiency stored and will take part in the matching.
   *  The minimal purity can be chosen by the minimalPurity parameter (default 2.0/3.0).
   *  The minimal efficiency can be chosen by the minimalEfficiency parameter (default 0.05).
   *
   *  Last but not least a RelationArray from matched PRTracks to MCParticles is build and
   *  the McTrackId property of the PRTrack is set to the StoreArray index of the MCParticle
   *  (similar as the MCTrackFinder does it for the MCTracks).
   *  By default clone tracks are also assigned to their MCParticle.
   *  This behaviour can be switched off by the relateClonesToMCParticles.
   *
   *  In the following a more detailed explanation is given for the matching and
   *  the classification of PRTracks and MCTracks.
   *
   *  The PRTracks can be classified into six categories:
   *
   *  - UNDEFINED
   *      - Status of the track *before* any real status has been set.
   *      - If the track status is still UNDEFINED after matching, something went wrong.
   *
   *  - MATCHED
   *      - The highest efficiency PRTrack of the highest purity MCTrack to this PRTrack is the same as this PRTrack.
   *      - In addition, the charge of the PRTrack and the one of the MCTrack are the same.
   *      - This means the PRTrack contains a high contribution of only one MCTrack and
   *        is also the best of all PRTracks describing this MCTrack (including the charge).
   *      - The McTrackId property of matched PRTrack is set to the MCTrackId property of the MCTrack,
   *        which is usually the index of the MCParticle in its corresponding StoreArray.
   *      - Also the relation from PRTrack to MCParticle is added.
   *      - The purity relation is setup from the PRTrack to the MCTrack with the (positive) purity as weight.
   *
   *  - WRONG CHARGE
   *      - The highest efficiency PRTrack of the highest purity MCTrack to this PRTrack is the same as this PRTrack.
   *      - But, the charge of the PRTrack and the one of the MCTrack are NOT the same.
   *      - This means the PRTrack contains a high contribution of only one MCTrack and
   *        is also the best of all PRTracks describing this MCTrack, but the charge is wrong.
   *      - The McTrackId property of matched PRTrack is set to the MCTrackId property of the MCTrack,
   *        which is usually the index of the MCParticle in its corresponding StoreArray.
   *      - Also the relation from PRTrack to MCParticle is added.
   *      - The purity relation is setup from the PRTrack to the MCTrack with the (positive) purity as weight.
   *
   *  - CLONE
   *      - The highest purity MCTrack has a different highest efficiency PRTrack than this track.
   *      - Anyway, the charge of the PRTrack and the one of the MCTrack are the same.
   *      - This means the PRTrack contains high contributions of only one MCTrack
   *        but a different other PRTrack contains an even higher contribution to this MCTrack.
   *      - Only if the relateClonesToMCParticles parameter is active
   *        The McTrackId property of cloned PRTracks is set to the MCTrackId property of the MCTrack.
   *        Else it will be set to -9.
   *      - Also the relation from PRTrack to MCParticle is added, only if the relateClonesToMCParticles parameter is active.
   *      - The purity relation is always setup from the PRTrack to the MCTrack with the _negative_ purity as weight,
   *        to be able to distinguish them from the matched tracks.
   *
   *  - CLONE WRONG CHARGE
   *      - The highest purity MCTrack has a different highest efficiency PRTrack than this track.
   *      - Moreover, the charge of the PRTrack and the one of the MCTrack are NOT the same.
   *      - This means the PRTrack contains high contributions of only one MCTrack
   *        but a different other PRTrack contains an even higher contribution to this MCTrack.
   *      - Only if the relateClonesToMCParticles parameter is active
   *        The McTrackId property of cloned PRTracks is set to the MCTrackId property of the MCTrack.
   *        Else it will be set to -9.
   *      - Also the relation from PRTrack to MCParticle is added, only if the relateClonesToMCParticles parameter is active.
   *      - The purity relation is always setup from the PRTrack to the MCTrack with the _negative_ purity as weight,
   *        to be able to distinguish them from the matched tracks.
   *
   *  - BACKGROUND
   *      - The PRTrack contains mostly hits, which are not part of any MCTrack.
   *      - This normally means, that this PRTracks is made of beam background hits.
   *      - If one constrains the MCTracks in the MCTrackFinder to some specific particles, say the tag side,
   *        also all signal side tracks end up in this category (in case of reasonable tracking performance).
   *        In this case the background rate is somewhat less meaningful.
   *      - For background tracks the McTrackId of the PRTrack is set to -99.
   *      - No relation from the PRTrack to the MCParticle is inserted.
   *      - PRTracks classified as background are not entered in the purity RelationArray.
   *
   *  - GHOST
   *      - The highest purity MCTrack to this PRTrack has a purity lower than the minimal purity given in the parameter minimalPurity or
   *      - has an efficiency lower than the efficiency given in the parameter minimalEfficiency.
   *      - This means that the PRTrack does not contain a significat number of a specific MCTrack nor can it considered only made of background.
   *      - For ghost tracks the McTrackId of the RecoTracks is set to -999.
   *      - No relation from the PRTrack to the MCParticle is inserted.
   *      - PRTracks classified as ghost are not entered in the purity RelationArray.
   *  .
   *
   *  MCTracks are classified into five categories:
   *
   *  - UNDEFINED
   *      - Status of the track *before* any real status has been set.
   *      - If the track status is still UNDEFINED after matching, something went wrong.
   *
   *  - MATCHED
   *      - The highest purity MCTrack of the highest efficiency PRTrack of this MCTrack is the same as this MCTrack.
   *      - In addition, the charge of the MCTrack and the one of the PRTrack are the same.
   *      - This means the MCTrack is well described by a PRTrack and this PRTrack has only a significant contribution from this MCTrack.
   *      - The efficiency relation is setup from the MCTrack to the PRTrack with the (positive) efficiency as weight.
   *
   *  - WRONG CHARGE
   *      - The highest purity MCTrack of the highest efficiency PRTrack of this MCTrack is the same as this MCTrack.
   *      - But, the charge of the MCTrack and the one of the PRTrack are NOT the same.
   *      - This means the MCTrack is well described by a PRTrack and this PRTrack has only a significant contribution from this MCTrack.
   *      - The efficiency relation is setup from the MCTrack to the PRTrack with the (positive) efficiency as weight.
   *
   *  - MERGED
   *      - The highest purity MCTrack of the highest efficiency PRTrack of this MCTrack is not the same as this MCTrack.
   *      - Anyway, the charge of this MCTrack and the one of the PRTrack are the same.
   *      - This means this MCTrack is mostly contained in a PRTrack, which in turn however better describes a MCTrack different form this.
   *      - The efficiency relation is setup from the MCTrack to the PRTrack with the _negative_ efficiency as weight,
   *        to be able to distinguish them from the matched tracks.
   *
   *  - MERGED WRONG CHARGE
   *      - The highest purity MCTrack of the highest efficiency PRTrack of this MCTrack is not the same as this MCTrack.
   *      - Moreover, the charge of this MCTrack and the one of the PRTrack are NOT the same.
   *      - This means this MCTrack is mostly contained in a PRTrack, which in turn however better describes a MCTrack different form this.
   *      - The efficiency relation is setup from the MCTrack to the PRTrack with the _negative_ efficiency as weight,
   *        to be able to distinguish them from the matched tracks.
   *
   *  - MISSING
   *      - There is no highest efficiency PRTrack to this MCTrack, which also fullfills the minimal purity requirement.
   *      - For this category no efficiency relation is inserted.
   *  .
   *
   */
  class MCRecoTracksMatcherModule : public Module {

  public:
    //! Constructor setting up the parameter
    MCRecoTracksMatcherModule();

    //! Signal the beginning of the event processing
    void initialize() final;

    //! Process the event
    void event() final;

  private: //Parameters
    //! Parameter : Name of the RecoTracks StoreArray from pattern recognition
    std::string m_prRecoTracksStoreArrayName;

    //! Parameter : Name of the RecoTracks StoreArray from MC track finding
    std::string m_mcRecoTracksStoreArrayName;

    //! Parameter : Name of the Tracks StoreArray
    std::string m_TracksStoreArrayName;

    //! Parameter : Switch whether PXDHits should be used in the matching
    bool m_usePXDHits;

    //! Parameter : Switch whether SVDHits should be used in the matching
    bool m_useSVDHits;

    //! Parameter : Switch whether CDCHits should be used in the matching
    bool m_useCDCHits;

    //! Parameter : Switch whether only axial CDCHits should be used
    bool m_useOnlyAxialCDCHits;

    //! Use fitted tracks for matching
    bool m_useFittedTracks = true;

    /*!
     *  Parameter : Minimal purity of a PRTrack to be considered matchable to a MCTrack.
     *
     *  This number encodes how many correct hits are minimally need to compensate for a false hits.
     *  The default 2. / 3. suggests that for each background hit can be compensated by two correct hits.
     */
    double m_minimalPurity;

    /*!
     *  Parameter : Minimal efficiency for a MCTrack to be considered matchable to a PRTrack
     *
     *  This number encodes which fraction of the true hits must at least be in the reconstructed track.
     *  The default 0.05 suggests that at least 5% of the true hits should have been picked up.
     */
    double m_minimalEfficiency;

    StoreArray<MCParticle>  m_MCParticles;  /**< StoreArray containing MCParticles */
    StoreArray<RecoTrack>   m_PRRecoTracks; /**< StoreArray containing PR RecoTracks */
    StoreArray<RecoTrack>   m_MCRecoTracks; /**< StoreArray containing MC RecoTracks */
    StoreArray<PXDCluster>  m_PXDClusters;  /**< StoreArray containing PXDClusters */
    StoreArray<SVDCluster>  m_SVDClusters;  /**< StoreArray containing SVDClusters */
    StoreArray<CDCHit>      m_CDCHits;      /**< StoreArray containing CDCHits */

    //! Flag to indicated whether the Monte Carlo track are on the DataStore
    bool m_mcParticlesPresent = false;

    //! Descriptive type defintion for a number of degrees of freedom.
    using NDF = int;

    //! Map storing the standard number degrees of freedom for a single hit by detector */
    std::map<int, NDF> m_ndf_by_detId = {{Const::PXD, 2}, {Const::SVD, 1}, {Const::CDC, 1}};
  };
}
