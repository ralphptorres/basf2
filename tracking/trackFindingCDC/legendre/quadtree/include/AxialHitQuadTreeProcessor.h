/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeProcessor.h>
#include <tracking/trackFindingCDC/legendre/precisionFunctions/PrecisionUtil.h>

#include <tracking/trackFindingCDC/numerics/LookupTable.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** A QuadTreeProcessor for TrackHits */
    class AxialHitQuadTreeProcessor : public QuadTreeProcessor<long, float, const CDCWireHit> {

    public:
      /**
       *  Get the standard lookup table containing equally spaces unit vectors (cos, sin)
       *
       *  It contains 2**16 + 1 sampling points between -pi and pi.
       */
      static const LookupTable<Vector2D>& getCosSinLookupTable();
      /**
       *  Constructs an array with the curvature bounds as generated by the default bin divisions
       */
      static std::vector<float> createCurvBound(YSpan curvSpan, int lastLevel);

    public:
      /// Constructor
      AxialHitQuadTreeProcessor(int lastLevel,
                                int seedLevel,
                                const XYSpans& ranges,
                                PrecisionUtil::PrecisionFunction precisionFunction);

      /**
       *  Constructor for the quad tree processor used in the off-origin extension.
       *  Currently only used in zero level mode to collect hits that are in a phase space part
       *  with respect to the given point.
       */
      AxialHitQuadTreeProcessor(const Vector2D& localOrigin,
                                const YSpan& curvSpan,
                                const LookupTable<Vector2D>* cosSinLookupTable);

    protected: // Section of specialized functions
      /**
       * lastLevel depends on curvature of the track candidate
       */
      bool isLeaf(QuadTree* node) const final;

      /**
       * Return the new ranges. We do not use the standard ranges for the lower levels.
       * @param node quadtree node
       * @param i theta index of the child
       * @param j rho index of the child
       * @return returns ranges of the (i;j) child
       */
      XYSpans createChild(QuadTree* node, int i, int j) const final;

      /**
       * Check whether hit belongs to the quadtree node:
       * @param node quadtree node
       * @param wireHit hit being checked
       * @return returns true if sinogram of the hit crosses (geometrically) borders of the node
       */
      bool isInNode(QuadTree* node, const CDCWireHit* wireHit) const final;

    protected: // Implementation details
      /**
       * Check derivative of the sinogram.
       * @param node QuadTree node
       * @param wireHit pointer to the hit to check
       * @return returns true in cases:
       * @return    - positive derivative and no extremum in the node's ranges or
       * @return    - extremum located in the node's ranges
       * @return returns false in other cases (namely negative derivative
       *
       */
      bool checkDerivative(QuadTree* node, const CDCWireHit* wireHit) const;

      /**
       * Checks whether extreme point is located within QuadTree node's ranges
       * @param node QuadTree node
       * @param wireHit hit to check
       * @return true or false
       */
      bool checkExtremum(QuadTree* node, const CDCWireHit* wireHit) const;

    public: // debug stuff
      /// Draw QuadTree node
      void drawHits(std::vector<const CDCWireHit*> hits, unsigned int color = 46) const;
      /// Draw QuadTree node
      void drawNode(QuadTree* node) const;

    private:
      /// Lambda which holds resolution function for the quadtree
      PrecisionUtil::PrecisionFunction m_precisionFunction;

      /// Local origin on which the phase space coordinates are centered
      Vector2D m_localOrigin;

      /// Pinned lookup table for precomputed cosine and sine values
      const LookupTable<Vector2D>* m_cosSinLookupTable;

      /// The curvature above which the trajectory is considered a curler.
      const double c_curlCurv = 0.02;

      /**
       *  Indicator whether the two sided phases space insertion check should be used
       *  This option should automatically split back to back tracks in the low curvature regions
       */
      bool m_twoSidedPhaseSpace;
    };
  }
}
