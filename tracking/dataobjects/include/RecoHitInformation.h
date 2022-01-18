/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <cdc/dataobjects/CDCHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDCluster.h>
#include <klm/dataobjects/eklm/EKLMAlignmentHit.h>
#include <klm/dataobjects/KLMHit2d.h>

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {
  /**
   * This class stores additional information to every CDC/SVD/PXD hit stored in a RecoTrack.
   * Every hit information belongs to a single hit and a single RecoTrack (stored with relation).
   * If one hit should belong to more than one track, you have to create more than one RecoHitInformation.
   *
   * The RecoHitInformation stores information on:
   *   - the TrackFinder that added the hit to the track
   *   - RL information (valid for for CDC hits only)
   *   - the sorting parameter of the hit. This is the index of this hit in the reco track.
   *   - additional flags
   *
   * The stored information can be used when transforming a RecoTrack into a genfit::Track or genfit::TrackCand
   */

  class RecoHitInformation : public RelationsObject {
  public:

    /** Define, use of clusters or true hits for SVD.
     *
     *  You have to decide, if you want to use Clusters or true hits at compile-time.
     *  In the real experiment, we want to use clusters without overhead from checking every time,
     *  if we should use true hits instead.
     */
    typedef SVDCluster UsedSVDHit;

    /** Define, use of clusters or true hits for PXD. */
    typedef PXDCluster UsedPXDHit;

    /** Define, use of CDC hits as CDC hits (for symmetry). */
    typedef CDCHit UsedCDCHit;

    /** Define, use of KLMHit2d as BKLM hits. */
    typedef KLMHit2d UsedBKLMHit;

    /** Define, use of EKLMHit2d as EKLM hits. */
    typedef EKLMAlignmentHit UsedEKLMHit;

    /** The RightLeft information of the hit which is only valid for CDC hits */
    enum RightLeftInformation {
      c_undefinedRightLeftInformation,
      c_invalidRightLeftInformation,
      c_right,
      c_left
    };

    /** The TrackFinder which has added the hit to the track */
    enum OriginTrackFinder {
      c_undefinedTrackFinder,
      // the Hit has been generated by the MCTrackFinder and its considered important
      // to find this hit by PR. Which hits get marked as priority and auxiliary solely
      // depends on the configuration of the TrackFinderMCRTruh
      c_MCTrackFinderPriorityHit,
      // the Hit has been generated by the MCTrackFinder and is considered to be of minor
      // importance to find, for example because it is in one of the downstream loops.
      // Which hits get marked as priority and auxiliary solely depends on the configuration
      // of the TrackFinderMCRTruh
      c_MCTrackFinderAuxiliaryHit,
      c_invalidTrackFinder,
      // non CKF-based track finders
      c_CDCTrackFinder,
      c_LocalTrackFinder,
      c_SegmentTrackCombiner,
      c_VXDTrackFinder,
      // track finders based on CKF
      c_SVDtoCDCCKF,
      c_ECLtoCDCCKF,
      c_CDCtoSVDCKF,
      c_SVDtoPXDCKF,
      // CDC hit finder implemented in the ReattachCDCWireHitsToRecoTracks module:
      // looks for CDC hits that are close to RecTracks, but that were rejected by the ADC/TOT based filter.
      c_ReattachCDCWireHitsToRecoTracks,
      // Belle I's track finder
      c_Trasan,
      c_other
    };

    /** Another flag to be used (currently unused) */
    enum RecoHitFlag {
      c_undefinedRecoHitFlag,
      c_dismissedByFit,
      c_pruned,
    };

    /** The detector this hit comes from (which is of course also visible in the hit type) */
    enum RecoHitDetector {
      c_undefinedTrackingDetector,
      c_invalidTrackingDetector,
      c_SVD,
      c_PXD,
      c_CDC,
      c_EKLM,
      c_BKLM
    };

  public:
    /**
     * Empty constructor for root.
     */
    RecoHitInformation() {}

    /**
     * Create hit information for a CDC hit with the given information. Adds the relation to the hit automatically.
     * @param cdcHit The hit to create this information for.
     * @param rightLeftInformation The RL-information.
     * @param foundByTrackFinder Which track finder has found this hit?
     * @param sortingParameter The sorting parameter of this hit.
     */
    RecoHitInformation(const UsedCDCHit* cdcHit, RightLeftInformation rightLeftInformation, OriginTrackFinder foundByTrackFinder,
                       unsigned int sortingParameter) :
      RecoHitInformation(cdcHit, RecoHitDetector::c_CDC, rightLeftInformation, foundByTrackFinder, sortingParameter)
    {
    }

    /**
     * Create hit information for a PXD hit with the given information. Adds the relation to the hit automatically.
     * @param pxdHit The hit to create this information for.
     * @param foundByTrackFinder Which track finder has found this hit?
     * @param sortingParameter The sorting parameter of this hit.
     */
    RecoHitInformation(const UsedPXDHit* pxdHit, OriginTrackFinder foundByTrackFinder,
                       unsigned int sortingParameter) :
      RecoHitInformation(pxdHit, RecoHitDetector::c_PXD, RightLeftInformation::c_invalidRightLeftInformation,
                         foundByTrackFinder, sortingParameter)
    {
    }

    /**
     * Create hit information for a SVD hit with the given information. Adds the relation to the hit automatically.
     * @param svdHit The hit to create this information for.
     * @param foundByTrackFinder Which track finder has found this hit?
     * @param sortingParameter The sorting parameter of this hit.
     */
    RecoHitInformation(const UsedSVDHit* svdHit, OriginTrackFinder foundByTrackFinder,
                       unsigned int sortingParameter) :
      RecoHitInformation(svdHit, RecoHitDetector::c_SVD, RightLeftInformation::c_invalidRightLeftInformation,
                         foundByTrackFinder, sortingParameter)
    {
    }

    /**
     * Create hit information for a EKLM hit with the given information. Adds the relation to the hit automatically.
     * @param eklmHit The hit to create this information for.
     * @param foundByTrackFinder Which track finder has found this hit?
     * @param sortingParameter The sorting parameter of this hit.
     */
    RecoHitInformation(const UsedEKLMHit* eklmHit, OriginTrackFinder foundByTrackFinder,
                       unsigned int sortingParameter) :
      RecoHitInformation(eklmHit, RecoHitDetector::c_EKLM, RightLeftInformation::c_invalidRightLeftInformation,
                         foundByTrackFinder, sortingParameter)
    {
    }

    /**
     * Create hit information for a BKLM hit with the given information. Adds the relation to the hit automatically.
     * @param bklmHit The hit to create this information for.
     * @param foundByTrackFinder Which track finder has found this hit?
     * @param sortingParameter The sorting parameter of this hit.
     */
    RecoHitInformation(const UsedBKLMHit* bklmHit, OriginTrackFinder foundByTrackFinder,
                       unsigned int sortingParameter) :
      RecoHitInformation(bklmHit, RecoHitDetector::c_BKLM, RightLeftInformation::c_invalidRightLeftInformation,
                         foundByTrackFinder, sortingParameter)
    {
    }

    /** Get the additional flag */
    RecoHitFlag getFlag() const
    {
      return m_flag;
    }

    /** Set the additional flag */
    void setFlag(RecoHitFlag flag)
    {
      m_flag = flag;
    }

    /** Get which track finder has found the track. */
    OriginTrackFinder getFoundByTrackFinder() const
    {
      return m_foundByTrackFinder;
    }

    /** Set which track finder has found the track. */
    void setFoundByTrackFinder(OriginTrackFinder foundByTrackFinder)
    {
      m_foundByTrackFinder = foundByTrackFinder;
    }

    /** Get the sorting parameter */
    unsigned int getSortingParameter() const
    {
      return m_sortingParameter;
    }

    /** Set the sorting parameter */
    void setSortingParameter(unsigned int sortingParameter)
    {
      m_sortingParameter = sortingParameter;
    }

    /** Get the right-left-information. */
    RightLeftInformation getRightLeftInformation() const
    {
      return m_rightLeftInformation;
    }

    /** Set the right-left-information. */
    void setRightLeftInformation(RightLeftInformation rightLeftInformation)
    {
      m_rightLeftInformation = rightLeftInformation;
    }

    /** Get the detector this hit comes from. (can not be changed once created) */
    RecoHitDetector getTrackingDetector() const
    {
      return m_trackingDetector;
    }

    /** Get the flag, whether this his should be used in a fit or not. */
    bool useInFit() const
    {
      return m_useInFit;
    }

    /** Set the hit to be used (default) or not in the next fit. */
    void setUseInFit(const bool useInFit = true)
    {
      m_useInFit = useInFit;
    }

    /**
     * Set the id of the created track point to the one from the genfit::Track.
     * This should only be used, if you really know what you are doing.
     */
    void setCreatedTrackPointID(int trackPointID)
    {
      m_createdTrackPointID = trackPointID;
    }

    /**
     * Get the id of the TrackPoint related to this reco hit information
     * in the genfit::Track.
     * Do not use this method unless you really know what you are doing.
     * Better, use the methods of the RecoTrack itself to retrieve
     * the TrackPoint directly.
     */
    int getCreatedTrackPointID() const
    {
      return m_createdTrackPointID;
    }

  private:
    /// The tracking detector this hit comes from (can not be changed once created)
    RecoHitDetector m_trackingDetector = RecoHitDetector::c_undefinedTrackingDetector;
    /// The right-left-information of the hit. Can be invalid (for VXD hits) or unknown.
    RightLeftInformation m_rightLeftInformation = RightLeftInformation::c_undefinedRightLeftInformation;

    /// The sorting parameter as an index.
    unsigned int m_sortingParameter = 0;
    /// Which track finder has found this hit and added it to the reco track.
    /// Can only be used if creating the RecoTrack in the track finder.
    OriginTrackFinder m_foundByTrackFinder = OriginTrackFinder::c_undefinedTrackFinder;
    /// An additional flag to be used.
    RecoHitFlag m_flag = RecoHitFlag::c_undefinedRecoHitFlag;
    /// Set this flag to falso to not create a measurement out of this hit
    bool m_useInFit = true;
    /**
     * The index for the created TrackPoint in the genfit::Track of the related RecoTrack.
     * Do not use this id, if you do not really know the consequences, but let the
     * RecoTrack handle the internals.
     */
    int m_createdTrackPointID = -1;

    /**
     * Create hit information for a generic hit with the given information. Adds the relation to the hit automatically.
     * @param hit the hit to create a reco hit information for.
     * @param trackingDetector The detector the hit comes from.
     * @param rightLeftInformation The right left information (can be invalid)
     * @param foundByTrackFinder Which track finder has found the hit.
     * @param sortingParameter The sorting parameter of the hit.
     */
    template <class HitType>
    RecoHitInformation(const HitType* hit,
                       RecoHitDetector trackingDetector,
                       RightLeftInformation rightLeftInformation,
                       OriginTrackFinder foundByTrackFinder,
                       unsigned int sortingParameter) :
      m_trackingDetector(trackingDetector),
      m_rightLeftInformation(rightLeftInformation),
      m_sortingParameter(sortingParameter),
      m_foundByTrackFinder(foundByTrackFinder),
      m_flag(RecoHitFlag::c_undefinedRecoHitFlag)
    {
      addRelationTo(hit);
    }

    ClassDef(RecoHitInformation, 6); /**< This class implements additional information for hits */
  };
}
