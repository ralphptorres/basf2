/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* DQM headers. */
#include <dqm/core/DQMHistAnalysis.h>

/* KLM headers. */
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/eklm/EKLMElementNumbers.h>


/* ROOT headers. */
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>
#include <TString.h>
#include <TText.h>
#include <TLine.h>

/* C++ headers. */
#include <string>

namespace Belle2 {

  /**
   * Analysis of KLM DQM histograms.
   */
  class DQMHistAnalysisKLM2Module final : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor.
     */
    DQMHistAnalysisKLM2Module();

    /**
     * Initializer.
     */
    void initialize() override final;

    /**
     * Called when entering a new run.
     */
    void beginRun() override final;

    /**
     * This method is called for each event.
     */
    void event() override final;

    /**
     * This method is called if the current run ends.
     */
    void endRun() override final;


  private:

    /**
     * Process histogram containing the efficiencies.
     * @param[in]  effHist  Histogram itself.
     * @param[in]  denominator Denominator for efficiency hist.
     * @param[in]  numerator Numerator for efficiency hist.
     * @param[in]  canvas Canvas of interest.
     */
    void processEfficiencyHistogram(TH1* effHist,  TH1* denominator, TH1* numerator, TCanvas* canvas);


    /**
     * Process histogram containing the number of hits in plane.
     * @param[in]  histName  Histogram name.
     * @param[in]  histogram Histogram itself.
     */
    void processPlaneHistogram(const std::string& histName, TH1* histogram);

    /**
     * Process 2D efficiency histograms.
     */
    void process2DEffHistogram(TH1* mainHist, TH1* refHist, TH2* planeHist, TH2* errHist, int layers, int sectors,
                               bool ratioPlot, int* pvcount, TCanvas* eff2dCanv);

    /** TLine for boundary in plane histograms. */
    TLine m_PlaneLine;

    /** TText for names in plane histograms. */
    TText m_PlaneText;

    /** Histogram from DQMInfo with run type. */
    TH1* m_RunType = NULL;

    /** String with run type. */
    TString m_RunTypeString;

    /** Run type flag for null runs. */
    bool m_IsPhysicsRun = false;

    /** Histogram for BKLM plane efficiency. */
    TH1* m_eff_bklm = NULL;

    /** BKLM plane efficiency canvas */
    TCanvas* m_c_eff_bklm = NULL;

    /** Histogram for EKLM plane efficiency. */
    TH1* m_eff_eklm = NULL;

    /** EKLM plane efficiency canvas */
    TCanvas* m_c_eff_eklm = NULL;

    /** Histogram for BKLM sector efficiency. */
    TH1* m_eff_bklm_sector = NULL;

    /** Histogram for BKLM sector efficiency. */
    TCanvas* m_c_eff_bklm_sector = NULL;

    /** Histogram for EKLM sector efficiency. */
    TH1* m_eff_eklm_sector = NULL;

    /** Histogram for EKLM sector efficiency. */
    TCanvas* m_c_eff_eklm_sector = NULL;

    /** Monitoring object. */
    MonitoringObject* m_monObj {};

    /** EKLM element numbers. */
    const EKLMElementNumbers* m_EklmElementNumbers;

    /** 2D layer-sector efficiency differences */

    /** reference file name*/
    std::string m_refFileName;

    /** reference file*/
    TFile* m_refFile = nullptr;

    // configurable plotting parameters
    /** alarm threshold*/
    float m_alarmThr = 0;
    /** min value of 2d shifter histogram*/
    float m_min = 0;
    /** max value of 2d shifer histogram*/
    float m_max = 2;
    /** enable ratio mode (true) or difference mode (false)*/
    bool m_ratio = true;

    // bklm
    /** reference bklm eff histogram*/
    TH1* m_ref_efficiencies_bklm = NULL;
    /** 2d bklm shifter histogram*/
    TH2* m_eff2d_bklm = NULL;
    /** 2d bklm shifter text*/
    TH2* m_err_bklm = NULL;
    /** 2d bklm shifter canvas*/
    TCanvas* m_c_eff2d_bklm = NULL;

    // eklm
    /** reference eklm eff histogram histogram*/
    TH1* m_ref_efficiencies_eklm = NULL;
    /** 2d eklm shifter histogram*/
    TH2* m_eff2d_eklm = NULL;
    /** 2d eklm shifter text*/
    TH2* m_err_eklm = NULL;
    /** 2d eklm shifter canvas*/
    TCanvas* m_c_eff2d_eklm = NULL;

    /** Name of histogram directory */
    std::string m_histogramDirectoryName;

    /** Minimal number of entries for delta histogram update. */
    double m_minEvents;

    /** Number of inefficient BKLM layers. */
    int m_nEffBKLMLayers;

    /** Number of inefficient EKLM Layers*/
    int m_nEffEKLMLayers;


  };

}
