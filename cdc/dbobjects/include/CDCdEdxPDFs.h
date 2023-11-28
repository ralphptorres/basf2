/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* basf2 headers. */
#include <framework/dbobjects/dEdxPDFs.h>

/* ROOT headers. */
#include <TH2F.h>

namespace Belle2 {

  /**
   * Specialized class for holding the CDC dE/dx PDFs.
   */
  class CDCdEdxPDFs: public dEdxPDFs {

  public:

    /**
     * Default constructor.
     */
    CDCdEdxPDFs() = default;

    /**
     * Returns the CDC dE/dx PDF for the given particle hypothesis.
     * @param hypothesis Particle hypothesis (as in Const::ChargedStable::c_SetSize)
     * @param truncated If true, returns the truncated dE/dx PDF
     */
    const TH2F* getCDCPDF(const int hypothesis, const bool truncated) const
    {
      return getPDF(hypothesis, truncated);
    }

  private:

    /** Class version for the ROOT streamer. */
    ClassDef(CDCdEdxPDFs, 1);

  };

}
