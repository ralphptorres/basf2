/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* basf2 headers. */
#include <framework/gearbox/Const.h>

/* C++ headers. */
#include <array>

/* ROOT headers. */
#include <TH2F.h>
#include <TObject.h>

namespace Belle2 {

  /**
   * Base class for holding the dE/dx PDFs.
   */

  class dEdxPDFs: public TObject {

  public:

    /**
     * Default constructor.
     */
    dEdxPDFs() = default;

    /**
     * Return the dE/dx PDF for the given particle hypothesis.
     * @param hypothesis Particle hypothesis (as in Const::ChargedStable::c_SetSize)
     * @param truncated If true, return the truncated dE/dx PDF
     */
    const TH2F* getPDF(const int hypothesis, const bool truncated) const
    {
      return truncated ? &m_dEdxPDFsTruncated.at(hypothesis) : &m_dEdxPDFs.at(hypothesis);
    }

    /**
     * Set the dE/dx PDF for the given particle hypothesis.
     * @param pdf dE/dx PDF as a pointer to the 2D histogram (`TH2F*`)
     * @param hypothesis Particle hypothesis (as in Const::ChargedStable::c_SetSize)
     * @param truncated If true, set the truncated dE/dx PDF
     */
    void setPDF(const TH2F* pdf, const int hypothesis, const bool truncated)
    {
      TH2F temp{};
      pdf->Copy(temp);
      if (truncated)
        m_dEdxPDFsTruncated.at(hypothesis) = temp;
      else
        m_dEdxPDFs.at(hypothesis) = temp;
    }

    /**
     * Set the dE/dx PDF for the given particle hypothesis.
     * @param pdf dE/dx PDF as a 2D histogram (`TH2F`)
     * @param hypothesis Particle hypothesis (as in Const::ChargedStable::c_SetSize)
     * @param truncated If true, set the truncated dE/dx PDF
     */
    void setPDF(const TH2F pdf, const int hypothesis, const bool truncated)
    {
      if (truncated)
        m_dEdxPDFsTruncated.at(hypothesis) = pdf;
      else
        m_dEdxPDFs.at(hypothesis) = pdf;
    }

    /**
     * Set the dE/dx PDF for all the particle hypothesis.
     * @param pdfs Array of dE/dx PDF as a 2D histogram (`TH2F`) for all the particle hypotheses
     * @param truncated If true, set the truncated dE/dx PDFs
     */
    void setPDFs(const std::array<TH2F, Const::ChargedStable::c_SetSize>& pdfs, const bool truncated)
    {
      if (truncated)
        m_dEdxPDFsTruncated = pdfs;
      else
        m_dEdxPDFs = pdfs;
    }

  private:

    /** Array of dE/dx PDFs for each particle hypothesis. */
    std::array<TH2F, Const::ChargedStable::c_SetSize> m_dEdxPDFs;

    /** Array of truncated dE/dx PDFs for each particle hypothesis. */
    std::array<TH2F, Const::ChargedStable::c_SetSize> m_dEdxPDFsTruncated;

    /** Class version for the ROOT streamer. */
    ClassDef(dEdxPDFs, 1);

  };

}
