/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef TRGEFFDQMANALYSISMODULE_H
#define TRGEFFDQMANALYSISMODULE_H

#pragma once

#ifdef _BELLE2_EPICS
// EPICS
// #include "cadef.h"
#endif

//DQM
#include <dqm/core/DQMHistAnalysis.h>


//ARICH
// #include <arich/utility/ARICHChannelHist.h>

#include <vector>

#include <TCanvas.h>
#include <TLine.h>
#include <TH2Poly.h>
#include <TEfficiency.h>

namespace Belle2 {

  /**
   * Make summary of data quality from reconstruction
   */
  class DQMHistAnalysisTRGEFFModule final : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor
     */
    DQMHistAnalysisTRGEFFModule();

    /**
     * Destructor
     */
    ~DQMHistAnalysisTRGEFFModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    void initialize() override final;

    /**
     * Event processor.
     */
    void event() override final;

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    void endRun() override final;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    void terminate() override final;

  protected:
    bool m_debug;/**<debug*/
    bool m_enableAlert;/**<Enable alert by base color of canvases*/

    /** Run type flag for physics runs. */
    bool m_IsPhysicsRun;
    /** Run type flag for cosmic runs. */
    bool m_IsCosmicRun;
    /** Run type flag for debug runs. */
    bool m_IsDebugRun;

    // TCanvas* m_cPhi_eff_psnecl_ftdf = nullptr;       /**<Canvas for TRGEFF efficiency, simplified one for CR shifter*/
    // TH1F* m_hPhi_eff_psnecl_ftdf    = nullptr;       /**<Histogram for TRGEFF efficiency, phi of f bit with ecl mask */

    /***********************************************************************
     * More details meaning of the following histograms could be found in  *
     * 'basf2/trg/gdl/modules/trggdlDQM/include/TRGEFFDQMModule.h'  and    *
     * 'basf2/trg/gdl/modules/trggdlDQM/src/TRGEFFDQMModule.cc'            *
     * The histograms of the mask bits and the object bit with mask bits.  *
     * The canvas and efficiency for the object bit.                       *
     ***********************************************************************/

    /** for CDC TRG, the histograms for phi of the ecl mask bits, the phi of f bit with ecl mask bits.
     *  And the canvas of phi and the efficiency of phi for f bit */
    TH1F* m_hPhi_psnecl             = nullptr;   /**< Histogram for TRGEFF, phi of f bit with ecl */
    TH1F* m_hPhi_psnecl_ftdf        = nullptr;   /**< Histogram for TRGEFF, phi of f bit with ecl mask */
    TCanvas* m_cPhi_eff             = nullptr;   /**< Canvas for TRGEFF efficiency, Phi */
    TEfficiency* m_hPhi_eff         = nullptr;   /**< the efficiency of Phi */

    /** for CDC TRG, the histograms for momentum pt of the ecl mask bits, the pt of f bit with ecl mask bits.
     *  And the canvas and the efficiency of pt for f bit */
    TH1F* m_hPt_psnecl              = nullptr;   /**< Pt of ecl mask bit */
    TH1F* m_hPt_psnecl_ftdf         = nullptr;   /**< Pt of f bit with ecl mask */
    TCanvas* m_cPt_eff              = nullptr;   /**< Canvas for TRGEFF efficiency, Pt */
    TEfficiency* m_hPt_eff          = nullptr;   /**< the efficiency of Pt for f bit */

    //  CDC TRG, remove the bhabha veto for f bit
    TH1F* m_nobha_hPt_psnecl        = nullptr;   /**< Pt of ecl mask bit */
    TH1F* m_nobha_hPt_psnecl_ftdf   = nullptr;   /**< remove the bha_veto, Pt of f bit with ecl mask */
    TCanvas* m_nobha_cPt_eff        = nullptr;   /**< remove the bha_veto,Canvas for TRGEFF efficiency, Pt */
    TEfficiency* m_nobha_hPt_eff    = nullptr;   /**< remove the bha_veto,the efficiency of Pt for f bit */

    // (t3>0 and !bhaveto and !veto),  for z bit
    /** for CDC TRG, the histograms for 3-momentum p with ecl mask bits,
     * p of z bit with ecl mask bits. And the canvas of p and the efficiency of p */
    TH1F* m_hP3_z_psnecl            = nullptr;  /**< p of ecl mask bit */
    TH1F* m_hP3_z_psnecl_ftdf       = nullptr;  /**< p of z bit with ecl mask */
    TCanvas* m_cP3_z_eff            = nullptr;  /**< Canvas for TRGEFF efficiency,  z bit */
    TEfficiency* m_hP3_z_eff        = nullptr;  /**< the efficiency of p for z bit */


    //  (t3>0 and !bhaveto and !veto), for y bit
    /** for CDC TRG, the histograms for 3-momentum p with ecl mask bits,
    * p of y bit with ecl mask bits. And the canvas of p and the efficiency of p */
    TH1F* m_hP3_y_psnecl            = nullptr;  /**< p of ecl mask bit */
    TH1F* m_hP3_y_psnecl_ftdf       = nullptr;  /**< p of y bit with ecl mask */
    TCanvas* m_cP3_y_eff            = nullptr;  /**< Canvas for TRGEFF efficiency,  y bit */
    TEfficiency* m_hP3_y_eff        = nullptr;  /**< the efficiency of p for y bit */

    // for CDC TRG, remove bhabha veto for z and y bit
    TH1F* m_nobha_hP3_z_psnecl      = nullptr;  /**< p of ecl mask bit */
    TH1F* m_nobha_hP3_z_psnecl_ftdf = nullptr;  /**< remove the bha_veto, p of z bit with ecl mask */
    TCanvas* m_nobha_cP3_z_eff      = nullptr;  /**< remove the bha_veto, canvas for TRGEFF efficiency,  z bit */
    TEfficiency* m_nobha_hP3_z_eff  = nullptr;  /**< remove the bha_veto, the efficiency of p for z bit */

    TH1F* m_nobha_hP3_y_psnecl      = nullptr;  /**< p of ecl mask bit */
    TH1F* m_nobha_hP3_y_psnecl_ftdf = nullptr;  /**< remove the bha_veto, p of y bit with ecl mask */
    TCanvas* m_nobha_cP3_y_eff      = nullptr;  /**< remove the bha_veto, canvas for TRGEFF efficiency,  y bit */
    TEfficiency* m_nobha_hP3_y_eff  = nullptr;  /**< remove the bha_veto, the efficiency of p for y bit */


    //-"fyo" with track deleta_phi distribution
    /** for CDC TRG, the histograms for the largest dphi in an event with ecl mask bits,
    * the largest dphi in an event dphi of fyo bit with ecl mask bits. And the canvas of dphi and the efficiency of dphi */
    TH1F* m_fyo_dphi_psnecl             = nullptr;  /**< dphi of ecl mask bit */
    TH1F* m_fyo_dphi_psnecl_ftdf        = nullptr;  /**< dphi of fyo bit with ecl mask */
    TCanvas* m_c_fyo_dphi_eff           = nullptr;  /**< canvas for TRGEFF efficiency,  fyo bit */
    TEfficiency* m_fyo_dphi_eff         = nullptr;  /**< the efficiency of dphi for fyo bit */

    // remove the bha_veto for fyo
    TH1F* m_nobha_fyo_dphi_psnecl       = nullptr;  /**< dphi of ecl mask bit */
    TH1F* m_nobha_fyo_dphi_psnecl_ftdf  = nullptr;  /**< remove the bha_veto, dphi of fyo bit with ecl mask */
    TCanvas* m_c_nobha_fyo_dphi_eff     = nullptr;  /**< remove the bha_veto, canvas for TRGEFF efficiency,  fyo bit */
    TEfficiency* m_nobha_fyo_dphi_eff   = nullptr;  /**< remove the bha_veto, the efficiency of dphi for fyo bit */

    //-"stt " with track p, θ, phi distribution
    /** for CDC TRG, the histograms for the phi distribution with ecl mask bits,
    * the phi of stt bit with ecl mask bits. And the canvas of phi and the efficiency of phi for the stt bit. */
    TH1F* m_stt_phi_psnecl              = nullptr;  /**< phi of ecl mask bit */
    TH1F* m_stt_phi_psnecl_ftdf         = nullptr;  /**< phi of stt bit with ecl mask */
    TCanvas* m_c_stt_phi_eff            = nullptr;  /**< canvas for TRGEFF efficiency,  stt bit */
    TEfficiency* m_stt_phi_eff          = nullptr;  /**< the efficiency of phi for stt bit */

    /** for CDC TRG, the histograms for the largest momentum p distribution in an event of the ecl mask bits,
    * the largest momentum p distribution in an event of stt bit with ecl mask bits.
    * And the canvas of stt and the efficiency of stt bit. */
    TH1F* m_stt_P3_psnecl               = nullptr;  /**< p of ecl mask bit */
    TH1F* m_stt_P3_psnecl_ftdf          = nullptr;  /**< p of stt bit with ecl mask */
    TCanvas* m_c_stt_P3_eff             = nullptr;  /**< canvas for TRGEFF efficiency,  stt bit */
    TEfficiency* m_stt_P3_eff           = nullptr;  /**< the efficiency of p for stt bit */

    /** for CDC TRG, the theta of stt distribution.
     * the histograms for the theta of the ecl mask bits,  and the theta of stt bit with ecl mask bits.
     * And the canvas and efficiency of the stt bit */
    TH1F* m_stt_theta_psnecl            = nullptr;  /**< theta of ecl mask bit */
    TH1F* m_stt_theta_psnecl_ftdf       = nullptr;  /**< theta of stt bit with ecl mask */
    TCanvas* m_c_stt_theta_eff          = nullptr;  /**< canvas for TRGEFF efficiency,  stt bit */
    TEfficiency* m_stt_theta_eff        = nullptr;  /**< the efficiency of theta for stt bit */

    // for CDC TRG, remove the bha_veto of the stt bit. (the distribution of phi, p and theta)
    TH1F* m_nobha_stt_phi_psnecl        = nullptr;  /**< phi of ecl mask bit */
    TH1F* m_nobha_stt_phi_psnecl_ftdf   = nullptr;  /**< remove the bha_veto, phi of stt bit with ecl mask */
    TCanvas* m_c_nobha_stt_phi_eff      = nullptr;  /**< remove the bha_veto, canvas for TRGEFF efficiency,  stt bit */
    TEfficiency* m_nobha_stt_phi_eff    = nullptr;  /**< remove the bha_veto, the efficiency of phi for stt bit */

    TH1F* m_nobha_stt_P3_psnecl         = nullptr;  /**< p of ecl mask bit */
    TH1F* m_nobha_stt_P3_psnecl_ftdf    = nullptr;  /**< remove the bha_veto, p of stt bit with ecl mask */
    TCanvas* m_c_nobha_stt_P3_eff       = nullptr;  /**< remove the bha_veto, canvas for TRGEFF efficiency,  stt bit */
    TEfficiency* m_nobha_stt_P3_eff     = nullptr;  /**< remove the bha_veto, the efficiency of p for stt bit */

    TH1F* m_nobha_stt_theta_psnecl      = nullptr;  /**< theta of ecl mask bit */
    TH1F* m_nobha_stt_theta_psnecl_ftdf = nullptr;  /**< remove the bha_veto, theta of stt bit with ecl mask */
    TCanvas* m_c_nobha_stt_theta_eff    = nullptr;  /**< remove the bha_veto, canvas for TRGEFF efficiency,  stt bit */
    TEfficiency* m_nobha_stt_theta_eff  = nullptr;  /**< remove the bha_veto, the efficiency of theta for stt bit */


    //-"hie" with ECL cluster energy sum distribution, as well as θ, phi.
    /** for ECL TRG, the histograms for the sum energy E distribution in an event within the thetaID range of the cdc mask bits,
     * and the sum E of the hie bit with cdc mask bits. And the canvas and the efficiency for the hie bit. */
    TH1F* m_hie_E_psnecl                = nullptr;   /**< E of cdc mask bit */
    TH1F* m_hie_E_psnecl_ftdf           = nullptr;   /**< E of hie bit with cdc mask */
    TCanvas* m_c_hie_E_eff              = nullptr;   /**< canvas for TRGEFF efficiency,  hie bit */
    TEfficiency* m_hie_E_eff            = nullptr;   /**< the efficiency of phi for hie bit */

    TH1F* m_hie_theta_psnecl            = nullptr;   /**< theta of cdc mask bit */
    TH1F* m_hie_theta_psnecl_ftdf       = nullptr;   /**< theta of hie bit with cdc mask */
    TCanvas* m_c_hie_theta_eff          = nullptr;   /**< canvas for TRGEFF efficiency,  hie bit */
    TEfficiency* m_hie_theta_eff        = nullptr;   /**< the efficiency of theta for hie bit */

    TH1F* m_hie_phi_psnecl              = nullptr;   /**< phi of cdc mask bit */
    TH1F* m_hie_phi_psnecl_ftdf         = nullptr;   /**< phi of hie bit with cdc mask */
    TCanvas* m_c_hie_phi_eff            = nullptr;   /**< canvas for TRGEFF efficiency,  hie bit */
    TEfficiency* m_hie_phi_eff          = nullptr;   /**< the efficiency of phi for hie bit */

    // remove the bha_veto for hie bit (the distribution of sum E, theta and phi)
    TH1F* m_nobha_hie_E_psnecl          = nullptr;  /**< E of cdc mask bit */
    TH1F* m_nobha_hie_E_psnecl_ftdf     = nullptr;  /**< remove the bha_veto, E of hie bit with cdc mask */
    TCanvas* m_c_nobha_hie_E_eff        = nullptr;  /**< remove the bha_veto, canvas for TRGEFF efficiency,  hie bit */
    TEfficiency* m_nobha_hie_E_eff      = nullptr;  /**< remove the bha_veto, the efficiency of phi for hie bit */

    TH1F* m_nobha_hie_theta_psnecl      = nullptr;  /**< theta of cdc mask bit */
    TH1F* m_nobha_hie_theta_psnecl_ftdf = nullptr;  /**< remove the bha_veto, theta of hie bit with cdc mask */
    TCanvas* m_c_nobha_hie_theta_eff    = nullptr;  /**< remove the bha_veto, canvas for TRGEFF efficiency,  hie bit */
    TEfficiency* m_nobha_hie_theta_eff  = nullptr;  /**< remove the bha_veto, the efficiency of theta for hie bit */

    TH1F* m_nobha_hie_phi_psnecl        = nullptr;  /**< phi of cdc mask bit */
    TH1F* m_nobha_hie_phi_psnecl_ftdf   = nullptr;  /**< remove the bha_veto, phi of hie bit with cdc mask */
    TCanvas* m_c_nobha_hie_phi_eff      = nullptr;  /**< remove the bha_veto, canvas for TRGEFF efficiency,  hie bit */
    TEfficiency* m_nobha_hie_phi_eff    = nullptr;  /**< remove the bha_veto, the efficiency of phi for hie bit */

    /** for ECL TRG, the histograms for the sum energy E distribution in an event with cdc mask bits,
     * the c1hie bit with cdc mask bits. And the canvas and the efficiency of c1hie bit. */
    TH1F* m_c1hie_E_psnecl              = nullptr;    /**< E of cdc mask bit */
    TH1F* m_c1hie_E_psnecl_ftdf         = nullptr;    /**< E of c1hie bit with cdc mask */
    TCanvas* m_c_c1hie_E_eff            = nullptr;    /**< canvas for TRGEFF efficiency,  c1hie bit */
    TEfficiency* m_c1hie_E_eff          = nullptr;    /**< the efficiency of phi for c1hie bit */

    // the theta distribution for the c1hie bit.
    TH1F* m_c1hie_theta_psnecl          = nullptr;    /**< theta of cdc mask bit */
    TH1F* m_c1hie_theta_psnecl_ftdf     = nullptr;    /**< theta of c1hie bit with cdc mask */
    TCanvas* m_c_c1hie_theta_eff        = nullptr;    /**< canvas for TRGEFF efficiency,  c1hie bit */
    TEfficiency* m_c1hie_theta_eff      = nullptr;    /**< the efficiency of theta for c1hie bit */

    // the phi distribution for the c1hie bit. the histograms of the cdc mask bits, the c1hie with cdc mask bits.
    TH1F* m_c1hie_phi_psnecl            = nullptr;    /**< phi of cdc mask bit */
    TH1F* m_c1hie_phi_psnecl_ftdf       = nullptr;    /**< phi of c1hie bit with cdc mask */
    TCanvas* m_c_c1hie_phi_eff          = nullptr;    /**< canvas for TRGEFF efficiency,  c1hie bit */
    TEfficiency* m_c1hie_phi_eff        = nullptr;    /**< the efficiency of phi for c1hie bit */


    /** for KLM TRG, the histograms for the phi of the ecl mask bits, and the phi of klmhit bit with ecl mask bits.
     * The canvas and efficiency for klmhit bit.*/
    // -"klmhit" with KLM cluster or track φ,θ distribution
    TH1F* m_klmhit_phi_psnecl           = nullptr;    /**< phi of ecl mask bit */
    TH1F* m_klmhit_phi_psnecl_ftdf      = nullptr;    /**< phi of klmhit bit with ecl mask */
    TCanvas* m_c_klmhit_phi_eff         = nullptr;    /**< canvas for TRGEFF efficiency,  klmhit bit */
    TEfficiency* m_klmhit_phi_eff       = nullptr;    /**< the efficiency of phi for klmhit bit */

    TH1F* m_klmhit_theta_psnecl         = nullptr;    /**< theta of ecl mask bit */
    TH1F* m_klmhit_theta_psnecl_ftdf    = nullptr;    /**< theta of klmhit bit with ecl mask */
    TCanvas* m_c_klmhit_theta_eff       = nullptr;    /**< canvas for TRGEFF efficiency,  klmhit bit */
    TEfficiency* m_klmhit_theta_eff     = nullptr;    /**< the efficiency of theta for klmhit bit */

    /** for KLM TRG, the histograms for the phi of the ecl mask bits, and the phi of eklmhit bit with ecl mask bits.
     * The canvas and efficiency of the eklmhit bit.*/
    TH1F* m_eklmhit_phi_psnecl          = nullptr;    /**< phi of ecl mask bit */
    TH1F* m_eklmhit_phi_psnecl_ftdf     = nullptr;    /**< phi of eklmhit bit with ecl mask */
    TCanvas* m_c_eklmhit_phi_eff        = nullptr;    /**< canvas for TRGEFF efficiency,  eklmhit bit */
    TEfficiency* m_eklmhit_phi_eff      = nullptr;    /**< the efficiency of phi for eklmhit bit */

    // the theta distribution for eklmhit bit
    TH1F* m_eklmhit_theta_psnecl        = nullptr;    /**< theta of ecl mask bit */
    TH1F* m_eklmhit_theta_psnecl_ftdf   = nullptr;    /**< theta of eklmhit bit with ecl mask */
    TCanvas* m_c_eklmhit_theta_eff      = nullptr;    /**< canvas for TRGEFF efficiency,  eklmhit bit */
    TEfficiency* m_eklmhit_theta_eff    = nullptr;    /**< the efficiency of theta for eklmhit bit */

    std::vector<std::pair<std::string, TH1F**>> histList; /**< the histogram list */
    std::vector<std::tuple<TH1F*, TH1F*, TEfficiency**>> efficiencyPairs; /**< the efficiency pair list */
    std::vector<std::tuple<TCanvas*, TEfficiency*>> canvasEfficiencyPairs; /**< the Canvas pair list */

    //mirabelle
    Belle2::MonitoringObject* mon_trgeff = nullptr; /**< MonitoringObject for trg */

  };

} // Belle2 namespace

#endif
