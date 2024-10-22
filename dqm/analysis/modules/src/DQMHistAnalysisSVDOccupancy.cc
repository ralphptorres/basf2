/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//+
// File : DQMHistAnalysisSVDOccupancy.cc
// Description : module for DQM histogram analysis of SVD sensors occupancies
//-


#include <dqm/analysis/modules/DQMHistAnalysisSVDOccupancy.h>
#include <vxd/geometry/GeoCache.h>

#include <TROOT.h>
#include <TStyle.h>
#include <TString.h>
#include <TAxis.h>

#include <TMath.h>
#include <iostream>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(DQMHistAnalysisSVDOccupancy);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

DQMHistAnalysisSVDOccupancyModule::DQMHistAnalysisSVDOccupancyModule()
  : DQMHistAnalysisSVDModule(false, true)
{
  //Parameter definition
  B2DEBUG(10, "DQMHistAnalysisSVDOccupancy: Constructor done.");

  setDescription("DQM Analysis Module that produces colored canvas for a straightforward interpretation of the SVD Data Quality.");

  addParam("occLevel_Error", m_occError, "Maximum Occupancy (%) allowed for safe operations (red)", double(5));
  addParam("occLevel_Warning", m_occWarning, "Occupancy (%) at WARNING level (orange)", double(3));
  addParam("occLevel_Empty", m_occEmpty, "Maximum Occupancy (%) for which the sensor is considered empty", double(0));
  addParam("onlineOccLevel_Error", m_onlineOccError, "Maximum OnlineOccupancy (%) allowed for safe operations (red)", double(10));
  addParam("onlineOccLevel_Warning", m_onlineOccWarning, "OnlineOccupancy (%) at WARNING level (orange)", double(5));
  addParam("onlineOccLevel_Empty", m_onlineOccEmpty, "Maximum OnlineOccupancy (%) for which the sensor is considered empty",
           double(0));
  addParam("printCanvas", m_printCanvas, "if True prints pdf of the analysis canvas", bool(false));
  addParam("additionalPlots", m_additionalPlots, "Flag to produce additional plots",   bool(false));
  addParam("samples3", m_3Samples, "if True 3 samples histograms analysis is performed", bool(false));
  addParam("PVPrefix", m_pvPrefix, "PV Prefix", std::string("SVD:"));
}

DQMHistAnalysisSVDOccupancyModule::~DQMHistAnalysisSVDOccupancyModule() { }

void DQMHistAnalysisSVDOccupancyModule::initialize()
{
  B2DEBUG(10, "DQMHistAnalysisSVDOccupancy: initialized.");

  const VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  //collect the list of all SVD Modules in the geometry here
  std::vector<VxdID> sensors = geo.getListOfSensors();
  for (VxdID& aVxdID : sensors) {
    VXD::SensorInfoBase info = geo.getSensorInfo(aVxdID);
    // B2INFO("VXD " << aVxdID);
    if (info.getType() != VXD::SensorInfoBase::SVD) continue;
    m_SVDModules.push_back(aVxdID); // reorder, sort would be better
  }
  std::sort(m_SVDModules.begin(), m_SVDModules.end());  // back to natural order

  //occupancy chart chip
  m_cOccupancyChartChip = new TCanvas("SVDOccupancy/c_OccupancyChartChip");

  //strip occupancy per sensor
  if (m_additionalPlots)
    m_sensors = m_SVDModules.size();
  else
    m_sensors = 2;

  m_cStripOccupancyU = new TCanvas*[m_sensors];
  m_cStripOccupancyV = new TCanvas*[m_sensors];
  for (int i = 0; i < m_sensors; i++) {
    int tmp_layer = m_SVDModules[i].getLayerNumber();
    int tmp_ladder = m_SVDModules[i].getLadderNumber();
    int tmp_sensor = m_SVDModules[i].getSensorNumber();
    m_cStripOccupancyU[i] = new TCanvas(Form("SVDOccupancy/c_StripOccupancyU_%d_%d_%d", tmp_layer, tmp_ladder, tmp_sensor));
    m_cStripOccupancyV[i] = new TCanvas(Form("SVDOccupancy/c_StripOccupancyV_%d_%d_%d", tmp_layer, tmp_ladder, tmp_sensor));
  }

  gROOT->cd();
  m_cOccupancyU = new TCanvas("SVDAnalysis/c_SVDOccupancyU");
  m_cOccupancyV = new TCanvas("SVDAnalysis/c_SVDOccupancyV");

  m_cOnlineOccupancyU = new TCanvas("SVDAnalysis/c_SVDOnlineOccupancyU");
  m_cOnlineOccupancyV = new TCanvas("SVDAnalysis/c_SVDOnlineOccupancyV");


  if (m_3Samples) {
    m_cOccupancyU3Samples = new TCanvas("SVDAnalysis/c_SVDOccupancyU3Samples");
    //  m_cOccupancyU->SetGrid(1);
    m_cOccupancyV3Samples = new TCanvas("SVDAnalysis/c_SVDOccupancyV3Samples");
    //  m_cOccupancyV->SetGrid(1);

    m_cOnlineOccupancyU3Samples = new TCanvas("SVDAnalysis/c_SVDOnlineOccupancyU3Samples");
    //  m_cOnlineOccupancyU->SetGrid(1);
    m_cOnlineOccupancyV3Samples = new TCanvas("SVDAnalysis/c_SVDOnlineOccupancyV3Samples");
    //  m_cOnlineOccupancyV->SetGrid(1);
  }

  m_cOccupancyUGroupId0 = new TCanvas("SVDAnalysis/c_SVDOccupancyUGroupId0");
  m_cOccupancyVGroupId0 = new TCanvas("SVDAnalysis/c_SVDOccupancyVGroupId0");

  m_hOccupancy =  new SVDSummaryPlots("hOccupancy@view", "Average OFFLINE Sensor Occupancy (%), @view/@side Side");
  m_hOccupancy->setStats(0);

  m_hOnlineOccupancy  =  new SVDSummaryPlots("hOnlineOccupancy@view", "Average ONLINE Sensor Occupancy (%), @view/@side Side");
  m_hOnlineOccupancy->setStats(0);

  m_hOccupancyGroupId0  =  new SVDSummaryPlots("hOccupancyGroupId0@view",
                                               "Average OFFLINE Sensor Occupancy (%), @view/@side Side for cluster time group Id = 0");
  m_hOccupancyGroupId0->setStats(0);

  if (m_3Samples) {
    m_hOccupancy3Samples  =  new SVDSummaryPlots("hOccupancy3@view",
                                                 "Average OFFLINE Sensor Occupancy (%), @view/@side Side for 3 samples");
    m_hOnlineOccupancy->setStats(0);

    m_hOnlineOccupancy3Samples  =  new SVDSummaryPlots("hOnlineOccupancy3@view",
                                                       "Average ONLINE Sensor Occupancy (%), @view/@side Side for 3 samples");
    m_hOnlineOccupancy3Samples->setStats(0);
  }

  //register limits for EPICS
  registerEpicsPV(m_pvPrefix + "occupancyLimits", "occLimits");
  registerEpicsPV(m_pvPrefix + "occupancyOnlineLimits", "occOnlineLimits");
}


void DQMHistAnalysisSVDOccupancyModule::beginRun()
{
  B2DEBUG(10, "DQMHistAnalysisSVDOccupancy: beginRun called.");
  m_cOccupancyU->Clear();
  m_cOccupancyV->Clear();

  m_cOnlineOccupancyU->Clear();
  m_cOnlineOccupancyV->Clear();
  m_cOccupancyChartChip->Clear();
  for (int i = 0; i < m_sensors; i++) {
    m_cStripOccupancyU[i]->Clear();
    m_cStripOccupancyV[i]->Clear();
  }

  if (m_3Samples) {
    m_cOccupancyU3Samples->Clear();
    m_cOccupancyV3Samples->Clear();
    m_cOnlineOccupancyU3Samples->Clear();
    m_cOnlineOccupancyV3Samples->Clear();
  }
  m_cOccupancyUGroupId0->Clear();
  m_cOccupancyVGroupId0->Clear();

  //Retrieve limits from EPICS
  double oocErrorLoOff = 0.;
  double oocErrorLoOn = 0.;
  double occWarningOff = 0.;
  double occWarningOn = 0.;

  requestLimitsFromEpicsPVs("occLimits", oocErrorLoOff, occWarningOff, m_occWarning,  m_occError);
  requestLimitsFromEpicsPVs("occOnlineLimits", oocErrorLoOn, occWarningOn, m_onlineOccWarning,  m_onlineOccError);

  B2DEBUG(10, " SVD occupancy thresholds taken from EPICS configuration file:");
  B2DEBUG(10, "  ONLINE OCCUPANCY: empty < " << m_onlineOccEmpty << " normal < " << m_onlineOccWarning << " warning < " <<
          m_onlineOccError <<
          " < error");
  B2DEBUG(10, "  OFFLINE OCCUPANCY: empty < " << m_occEmpty << " normal < " << m_occWarning << " warning < " << m_occError <<
          " < error with minimum statistics of " << m_occEmpty);

  // Create text panel
  //OFFLINE occupancy plots legend
  m_legProblem->Clear();
  m_legProblem->AddText("ERROR!");
  m_legProblem->AddText("at least one sensor with:");
  m_legProblem->AddText(Form("occupancy > %1.1f%%", m_occError));

  m_legWarning->Clear();
  m_legWarning->AddText("WARNING!");
  m_legWarning->AddText("at least one sensor with:");
  m_legWarning->AddText(Form("%1.1f%% < occupancy < %1.1f%%", m_occWarning, m_occError));

  m_legNormal->Clear();
  m_legNormal->AddText("OCCUPANCY WITHIN LIMITS");
  m_legNormal->AddText(Form("%1.1f%% < occupancy < %1.1f%%", m_occEmpty, m_occWarning));

  m_legEmpty->Clear();
  m_legEmpty->AddText("NO DATA RECEIVED");
  m_legEmpty->AddText("from at least one sensor");

  //ONLINE occupancy plots legend
  m_legOnProblem->Clear();
  m_legOnProblem->AddText("ERROR!");
  m_legOnProblem->AddText("at least one sensor with:");
  m_legOnProblem->AddText(Form("online occupancy > %1.1f%%", m_onlineOccError));

  m_legOnWarning->Clear();
  m_legOnWarning->AddText("WARNING!");
  m_legOnWarning->AddText("at least one sensor with:");
  m_legOnWarning->AddText(Form("%1.1f%% < online occupancy < %1.1f%%", m_onlineOccWarning, m_onlineOccError));

  m_legOnNormal->Clear();
  m_legOnNormal->AddText("OCCUPANCY WITHIN LIMITS");
  m_legOnNormal->AddText(Form("%1.1f%% < online occupancy < %1.1f%%", m_onlineOccEmpty, m_onlineOccWarning));
}

void DQMHistAnalysisSVDOccupancyModule::event()
{
  B2DEBUG(10, "DQMHistAnalysisSVDOccupancy: event called.");

  //find nEvents
  TH1* hnEvnts = findHist("SVDExpReco/SVDDQM_nEvents", true);
  if (hnEvnts == NULL) {
    B2INFO("no events, nothing to do here");
    return;
  } else {
    B2DEBUG(10, "SVDExpReco/SVDDQM_nEvents found");
  }

  TString tmp = hnEvnts->GetTitle();
  Int_t pos = tmp.Last('~');
  if (pos == -1) pos = 0;

  TString runID = tmp(pos, tmp.Length() - pos);
  B2INFO("DQMHistAnalysisSVDOccupancyModule::runID = " << runID);
  Float_t nEvents = hnEvnts->GetEntries();

  //occupancy chart
  TH1F* hChart = (TH1F*)findHist("SVDExpReco/SVDDQM_StripCountsChip");

  if (hChart != NULL) {
    m_hOccupancyChartChip.Clear();
    hChart->Copy(m_hOccupancyChartChip);
    m_hOccupancyChartChip.SetName("SVDOccupancyChart");
    m_hOccupancyChartChip.SetTitle(Form("SVD OFFLINE Occupancy per chip %s", runID.Data()));
    m_hOccupancyChartChip.Scale(1 / nEvents / 128);
    m_cOccupancyChartChip->cd();
    //    m_hOccupancyChartChip->SetStats(0);
    m_hOccupancyChartChip.Draw();
  }
  m_cOccupancyChartChip->Modified();
  m_cOccupancyChartChip->Update();

  if (m_printCanvas)
    m_cOccupancyChartChip->Print("c_OccupancyChartChip.pdf");


  //check MODULE OCCUPANCY online & offline
  //reset canvas color
  m_occUstatus = 0;
  m_occVstatus = 0;
  m_onlineOccUstatus = 0;
  m_onlineOccVstatus = 0;

  m_onlineOccU3Samples = 0;
  m_onlineOccV3Samples = 0;

  m_occU3Samples = 0;
  m_occV3Samples = 0;

  //update titles with exp and run number

  m_hOccupancy->reset();
  m_hOccupancy->setStats(0);
  m_hOccupancy->setRunID(runID);

  m_hOnlineOccupancy->reset();
  m_hOnlineOccupancy->setStats(0);
  m_hOnlineOccupancy->setRunID(runID);

  m_hOccupancyGroupId0->reset();
  m_hOccupancyGroupId0->setStats(0);
  m_hOccupancyGroupId0->setRunID(runID);

  if (m_3Samples) {
    m_hOccupancy3Samples->reset();
    m_hOccupancy3Samples->setStats(0);
    m_hOccupancy3Samples->setRunID(runID);

    m_hOnlineOccupancy3Samples->reset();
    m_hOnlineOccupancy3Samples->setStats(0);
    m_hOnlineOccupancy3Samples->setRunID(runID);
  }

  //set dedicate gStyle
  const Int_t colNum = 4;
  Int_t palette[colNum] {kBlack,  c_ColorGood, c_ColorWarning, c_ColorError};
  gStyle->SetPalette(colNum, palette);
  gStyle->SetOptStat(0);
  gStyle->SetPaintTextFormat("2.3f");

  TH1F* htmp = NULL;

  for (unsigned int i = 0; i < m_SVDModules.size(); i++) {
    int tmp_layer = m_SVDModules[i].getLayerNumber();
    int tmp_ladder = m_SVDModules[i].getLadderNumber();
    int tmp_sensor = m_SVDModules[i].getSensorNumber();

    //look for U histogram - OFFLINE ZS
    TString tmpname = Form("SVDExpReco/SVDDQM_%d_%d_%d_StripCountU", tmp_layer, tmp_ladder, tmp_sensor);

    htmp = (TH1F*)findHist(tmpname.Data());
    if (htmp == NULL) {
      B2INFO("Occupancy U histogram not found");
      m_cOccupancyU->Draw();
      m_cOccupancyU->cd();
      m_hOccupancy->getHistogram(1)->Draw("text");
      colorizeCanvas(m_cOccupancyU, c_StatusDefault);
    } else {

      Int_t nStrips = 768;

      Float_t occU = htmp->GetEntries() / nStrips / nEvents * 100;
      m_hOccupancy->fill(m_SVDModules[i], 1, occU);
      setOccStatus(occU, m_occUstatus);

      //produce the occupancy plot
      if (m_additionalPlots) {
        m_hStripOccupancyU[i].Clear();
        htmp->Copy(m_hStripOccupancyU[i]);
        m_hStripOccupancyU[i].Scale(1 / nEvents);
        m_hStripOccupancyU[i].SetName(Form("%d_%d_%d_OccupancyU", tmp_layer, tmp_ladder, tmp_sensor));
        m_hStripOccupancyU[i].SetTitle(Form("SVD Sensor %d_%d_%d U-Strip OFFLINE Occupancy vs Strip Number %s", tmp_layer, tmp_ladder,
                                            tmp_sensor, runID.Data()));
      } else {
        if (i == 0 || i == 1) {
          m_hStripOccupancyU[i].Clear();
          htmp->Copy(m_hStripOccupancyU[i]);
          m_hStripOccupancyU[i].Scale(1 / nEvents);
          m_hStripOccupancyU[i].SetName(Form("%d_%d_%d_OccupancyU", tmp_layer, tmp_ladder, tmp_sensor));
          m_hStripOccupancyU[i].SetTitle(Form("SVD Sensor %d_%d_%d U-Strip OFFLINE Occupancy vs Strip Number %s", tmp_layer, tmp_ladder,
                                              tmp_sensor, runID.Data()));
        }
      }
    }

    if (m_3Samples) {
      //look for U histogram - OFFLINE ZS for 3 samples
      tmpname = Form("SVDExpReco/SVDDQM_%d_%d_%d_Strip3CountU", tmp_layer, tmp_ladder, tmp_sensor);

      htmp = (TH1F*)findHist(tmpname.Data());
      if (htmp == NULL) {
        B2INFO("Occupancy U histogram not found for 3 samples");
        m_cOccupancyU3Samples->Draw();
        m_cOccupancyU3Samples->cd();
        m_hOccupancy3Samples->getHistogram(1)->Draw("text");
        colorizeCanvas(m_cOccupancyU3Samples, c_StatusDefault);
      } else {

        Int_t nStrips = 768;

        Float_t occU = htmp->GetEntries() / nStrips / nEvents * 100;
        m_hOccupancy3Samples->fill(m_SVDModules[i], 1, occU);
        setOccStatus(occU, m_occU3Samples);
      }
    }

    // groupId0 side U
    TString tmpnameGrpId0 = Form("SVDExpReco/SVDDQM_%d_%d_%d_StripCountGroupId0U", tmp_layer, tmp_ladder, tmp_sensor);
    htmp = (TH1F*)findHist(tmpnameGrpId0.Data());
    if (htmp == NULL) {
      B2INFO("Occupancy U histogram for group Id0 not found");
      m_cOccupancyUGroupId0->Draw();
      m_cOccupancyUGroupId0->cd();
      m_hOccupancyGroupId0->getHistogram(1)->Draw("text");
      colorizeCanvas(m_cOccupancyUGroupId0, c_StatusDefault);
    } else {

      Int_t nStrips = 768;

      Float_t occU = htmp->GetEntries() / nStrips / nEvents * 100;
      m_hOccupancyGroupId0->fill(m_SVDModules[i], 1, occU);
      setOccStatus(occU, m_occUGroupId0);
    }

    //look for V histogram - OFFLINE ZS
    tmpname = Form("SVDExpReco/SVDDQM_%d_%d_%d_StripCountV", tmp_layer, tmp_ladder, tmp_sensor);

    htmp = (TH1F*)findHist(tmpname.Data());
    if (htmp == NULL) {
      B2INFO("Occupancy V histogram not found");
      m_cOccupancyV->Draw();
      m_cOccupancyV->cd();
      m_hOccupancy->getHistogram(0)->Draw("text");
      colorizeCanvas(m_cOccupancyV, c_StatusDefault);
    } else {

      Int_t nStrips = 768;
      if (tmp_layer != 3)
        nStrips = 512;

      Float_t occV = htmp->GetEntries() / nStrips / nEvents * 100;
      m_hOccupancy->fill(m_SVDModules[i], 0, occV);
      setOccStatus(occV, m_occVstatus);

      //produce the occupancy plot
      if (m_additionalPlots) {
        m_hStripOccupancyV[i].Clear();
        htmp->Copy(m_hStripOccupancyV[i]);
        m_hStripOccupancyV[i].Scale(1 / nEvents);
        m_hStripOccupancyV[i].SetName(Form("%d_%d_%d_OccupancyV", tmp_layer, tmp_ladder, tmp_sensor));
        m_hStripOccupancyV[i].SetTitle(Form("SVD Sensor %d_%d_%d V-Strip OFFLINE Occupancy vs Strip Number %s", tmp_layer, tmp_ladder,
                                            tmp_sensor, runID.Data()));
      } else {
        if (i < (unsigned int)m_sensors) {
          m_hStripOccupancyV[i].Clear();
          htmp->Copy(m_hStripOccupancyV[i]);
          m_hStripOccupancyV[i].Scale(1 / nEvents);
          m_hStripOccupancyV[i].SetName(Form("%d_%d_%d_OccupancyV", tmp_layer, tmp_ladder, tmp_sensor));
          m_hStripOccupancyV[i].SetTitle(Form("SVD Sensor %d_%d_%d V-Strip OFFLINE Occupancy vs Strip Number %s", tmp_layer, tmp_ladder,
                                              tmp_sensor, runID.Data()));
        }
      }
    }

    if (m_3Samples) {
      //look for V histogram - OFFLINE ZS for 3 samples
      tmpname = Form("SVDExpReco/SVDDQM_%d_%d_%d_Strip3CountV", tmp_layer, tmp_ladder, tmp_sensor);

      htmp = (TH1F*)findHist(tmpname.Data());
      if (htmp == NULL) {
        B2INFO("Occupancy V histogram not found");
        m_cOccupancyV3Samples->Draw();
        m_cOccupancyV3Samples->cd();
        m_hOccupancy3Samples->getHistogram(0)->Draw("text");
        colorizeCanvas(m_cOccupancyV3Samples, c_StatusDefault);
      } else {

        Int_t nStrips = 768;
        if (tmp_layer != 3)
          nStrips = 512;

        Float_t occV = htmp->GetEntries() / nStrips / nEvents * 100;
        m_hOccupancy3Samples->fill(m_SVDModules[i], 0, occV);
        setOccStatus(occV, m_occV3Samples);
      }
    }

    // groupId0 side V
    tmpnameGrpId0 = Form("SVDExpReco/SVDDQM_%d_%d_%d_StripCountGroupId0V", tmp_layer, tmp_ladder, tmp_sensor);

    htmp = (TH1F*)findHist(tmpnameGrpId0.Data());
    if (htmp == NULL) {
      B2INFO("Occupancy U histogram for group Id0 not found");
      m_cOccupancyVGroupId0->Draw();
      m_cOccupancyVGroupId0->cd();
      m_hOccupancyGroupId0->getHistogram(0)->Draw("text");
      colorizeCanvas(m_cOccupancyVGroupId0, c_StatusDefault);
    } else {

      Int_t nStrips = 768;
      if (tmp_layer != 3)
        nStrips = 512;

      Float_t occV = htmp->GetEntries() / nStrips / nEvents * 100;
      m_hOccupancyGroupId0->fill(m_SVDModules[i], 0, occV);
      setOccStatus(occV, m_occVGroupId0);
    }

    //look for V histogram - ONLINE ZS
    tmpname = Form("SVDExpReco/SVDDQM_%d_%d_%d_OnlineZSStripCountV", tmp_layer, tmp_ladder, tmp_sensor);

    htmp = (TH1F*)findHist(tmpname.Data());
    if (htmp == NULL) {
      B2INFO("OnlineOccupancy V histogram not found");
      m_cOnlineOccupancyV->Draw();
      m_cOnlineOccupancyV->cd();
      m_hOnlineOccupancy->getHistogram(0)->Draw("text");
      colorizeCanvas(m_cOnlineOccupancyV, c_StatusDefault);
    } else {

      Int_t nStrips = 768;
      if (tmp_layer != 3)
        nStrips = 512;

      Float_t onlineOccV = htmp->GetEntries() / nStrips / nEvents * 100;
      m_hOnlineOccupancy->fill(m_SVDModules[i], 0, onlineOccV);


      for (int b = 1; b < htmp->GetNbinsX() + 1; b++) {
        htmp->SetBinContent(b, htmp->GetBinContent(b) / nEvents * 100);
      }
      htmp->GetYaxis()->SetTitle("ZS3 ccupancy (%)");

      setOccStatus(onlineOccV, m_onlineOccVstatus);
    }

    if (m_3Samples) {
      //look for V histogram - ONLINE ZS for 3 samples
      tmpname = Form("SVDExpReco/SVDDQM_%d_%d_%d_OnlineZSStrip3CountV", tmp_layer, tmp_ladder, tmp_sensor);

      htmp = (TH1F*)findHist(tmpname.Data());
      if (htmp == NULL) {
        B2INFO("OnlineOccupancy3 V histogram not found");
        m_cOnlineOccupancyV3Samples->Draw();
        m_cOnlineOccupancyV3Samples->cd();
        m_hOnlineOccupancy3Samples->getHistogram(0)->Draw("text");
        colorizeCanvas(m_cOnlineOccupancyV3Samples, c_StatusDefault);
      } else {

        Int_t nStrips = 768;
        if (tmp_layer != 3)
          nStrips = 512;

        Float_t onlineOccV = htmp->GetEntries() / nStrips / nEvents * 100;
        m_hOnlineOccupancy3Samples->fill(m_SVDModules[i], 0, onlineOccV);

        for (int b = 1; b < htmp->GetNbinsX() + 1; b++) {
          htmp->SetBinContent(b, htmp->GetBinContent(b) / nEvents * 100);
        }
        htmp->GetYaxis()->SetTitle("ZS3 ccupancy (%)");
        setOccStatus(onlineOccV, m_onlineOccV3Samples);
      }
    }

    //look for U histogram - ONLINE ZS
    tmpname = Form("SVDExpReco/SVDDQM_%d_%d_%d_OnlineZSStripCountU", tmp_layer, tmp_ladder, tmp_sensor);

    htmp = (TH1F*)findHist(tmpname.Data());
    if (htmp == NULL) {
      B2INFO("OnlineOccupancy U histogram not found");
      m_cOnlineOccupancyU->Draw();
      m_cOnlineOccupancyU->cd();
      m_hOnlineOccupancy->getHistogram(1)->Draw("text");
      colorizeCanvas(m_cOnlineOccupancyU, c_StatusDefault);
    } else {

      Int_t nStrips = 768;

      Float_t onlineOccU = htmp->GetEntries() / nStrips / nEvents * 100;
      m_hOnlineOccupancy->fill(m_SVDModules[i], 1, onlineOccU);

      for (int b = 1; b < htmp->GetNbinsX() + 1; b++) {
        htmp->SetBinContent(b, htmp->GetBinContent(b) / nEvents * 100);
      }
      htmp->GetYaxis()->SetTitle("ZS3 ccupancy (%)");
      setOccStatus(onlineOccU, m_onlineOccUstatus);
    }

    if (m_3Samples) {
      //look for U histogram - ONLINE ZS for 3 samples
      tmpname = Form("SVDExpReco/SVDDQM_%d_%d_%d_OnlineZSStrip3CountU", tmp_layer, tmp_ladder, tmp_sensor);

      htmp = (TH1F*)findHist(tmpname.Data());
      if (htmp == NULL) {
        B2INFO("OnlineOccupancy3 U histogram not found");
        m_cOnlineOccupancyU3Samples->Draw();
        m_cOnlineOccupancyU3Samples->cd();
        m_hOnlineOccupancy3Samples->getHistogram(1)->Draw("text");
        colorizeCanvas(m_cOnlineOccupancyU3Samples, c_StatusDefault);
      } else {

        Int_t nStrips = 768;

        Float_t onlineOccU = htmp->GetEntries() / nStrips / nEvents * 100;
        m_hOnlineOccupancy3Samples->fill(m_SVDModules[i], 1, onlineOccU);

        for (int b = 1; b < htmp->GetNbinsX() + 1; b++) {
          htmp->SetBinContent(b, htmp->GetBinContent(b) / nEvents * 100);
        }
        htmp->GetYaxis()->SetTitle("ZS3 ccupancy (%)");
        setOccStatus(onlineOccU, m_onlineOccU3Samples);
      }
    }

    //update sensor occupancy canvas U and V
    if (m_additionalPlots) {
      m_cStripOccupancyU[i]->cd();
      m_hStripOccupancyU[i].Draw("histo");
      m_cStripOccupancyV[i]->cd();
      m_hStripOccupancyV[i].Draw("histo");
    } else {
      if (i == 0 || i == 1) {
        m_cStripOccupancyU[i]->cd();
        m_hStripOccupancyU[i].Draw("histo");
        m_cStripOccupancyV[i]->cd();
        m_hStripOccupancyV[i].Draw("histo");
      }
    }
  }

  //update summary offline occupancy U canvas
  m_cOccupancyU->Draw();
  m_cOccupancyU->cd();
  m_hOccupancy->getHistogram(1)->Draw("text");
  setStatusOfCanvas(m_occUstatus, m_cOccupancyU);

  if (m_3Samples) {
    //update summary offline occupancy U canvas for 3 samples
    m_cOccupancyU3Samples->Draw();
    m_cOccupancyU3Samples->cd();
    m_hOccupancy3Samples->getHistogram(1)->Draw("text");
    setStatusOfCanvas(m_occU3Samples, m_cOccupancyU3Samples);
  }

  //update summary offline occupancy U canvas for groupId0
  m_cOccupancyUGroupId0->Draw();
  m_cOccupancyUGroupId0->cd();
  m_hOccupancyGroupId0->getHistogram(1)->Draw("text");
  setStatusOfCanvas(m_occUGroupId0, m_cOccupancyUGroupId0);

  //update summary offline occupancy V canvas
  m_cOccupancyV->Draw();
  m_cOccupancyV->cd();
  m_hOccupancy->getHistogram(0)->Draw("text");
  setStatusOfCanvas(m_occVstatus, m_cOccupancyV);

  if (m_3Samples) {
    //update summary offline occupancy V canvas for 3 samples
    m_cOccupancyV3Samples->Draw();
    m_cOccupancyV3Samples->cd();
    m_hOccupancy3Samples->getHistogram(0)->Draw("text");
    setStatusOfCanvas(m_occV3Samples, m_cOccupancyV3Samples);
  }

  //update summary offline occupancy V canvas for groupId0
  m_cOccupancyVGroupId0->Draw();
  m_cOccupancyVGroupId0->cd();
  m_hOccupancyGroupId0->getHistogram(0)->Draw("text");
  setStatusOfCanvas(m_occVGroupId0, m_cOccupancyVGroupId0);

  //update summary online occupancy U canvas
  m_cOnlineOccupancyU->Draw();
  m_cOnlineOccupancyU->cd();
  m_hOnlineOccupancy->getHistogram(1)->Draw("text");
  setStatusOfCanvas(m_onlineOccUstatus, m_cOnlineOccupancyU, true, true);

  //update summary online occupancy V canvas
  m_cOnlineOccupancyV->Draw();
  m_cOnlineOccupancyV->cd();
  m_hOnlineOccupancy->getHistogram(0)->Draw("text");
  setStatusOfCanvas(m_onlineOccVstatus, m_cOnlineOccupancyV, true, true);

  if (m_printCanvas) {
    m_cOccupancyU->Print("c_SVDOccupancyU.pdf");
    m_cOccupancyV->Print("c_SVDOccupancyV.pdf");
    m_cOnlineOccupancyU->Print("c_SVDOnlineOccupancyU.pdf");
    m_cOnlineOccupancyV->Print("c_SVDOnlineOccupancyV.pdf");
  }

  if (m_3Samples) {
    //update summary online occupancy U canvas for 3 samples
    m_cOnlineOccupancyU3Samples->Draw();
    m_cOnlineOccupancyU3Samples->cd();
    m_hOnlineOccupancy3Samples->getHistogram(1)->Draw("text");
    setStatusOfCanvas(m_onlineOccU3Samples, m_cOnlineOccupancyU3Samples, true, true);

    //update summary online occupancy V canvas for 3 samples
    m_cOnlineOccupancyV3Samples->Draw();
    m_cOnlineOccupancyV3Samples->cd();
    m_hOnlineOccupancy3Samples->getHistogram(0)->Draw("text");
    setStatusOfCanvas(m_onlineOccV3Samples, m_cOnlineOccupancyV3Samples, true, true);
  }
}

void DQMHistAnalysisSVDOccupancyModule::endRun()
{
  B2DEBUG(10, "DQMHistAnalysisSVDOccupancy: endRun called");
}


void DQMHistAnalysisSVDOccupancyModule::terminate()
{
  B2DEBUG(10, "DQMHistAnalysisSVDOccupancy: terminate called");

  delete m_cOccupancyU;
  delete m_cOccupancyV;

  delete m_hOccupancy;
  delete m_hOnlineOccupancy;
  delete m_hOccupancyGroupId0;
  delete m_hOccupancy3Samples;
  delete m_hOnlineOccupancy3Samples;

  delete m_cOnlineOccupancyU;
  delete m_cOnlineOccupancyV;

  delete m_cOccupancyChartChip;

  for (int module = 0; module < m_sensors; module++) {
    delete m_cStripOccupancyU[module];
    delete m_cStripOccupancyV[module];
  }
  delete m_cStripOccupancyU;
  delete m_cStripOccupancyV;
}

Int_t DQMHistAnalysisSVDOccupancyModule::findBinY(Int_t layer, Int_t sensor)
{
  if (layer == 3)
    return sensor; //2
  if (layer == 4)
    return 2 + 1 + sensor; //6
  if (layer == 5)
    return 6 + 1 + sensor; // 11
  if (layer == 6)
    return 11 + 1 + sensor; // 17
  else
    return -1;
}

void DQMHistAnalysisSVDOccupancyModule::setOccStatus(float occupancy, int& ocStatus, bool online)
{
  if (online) {
    if (occupancy <= m_onlineOccEmpty) {
      if (ocStatus < lowStat) ocStatus = lowStat;
    } else if (occupancy > m_onlineOccWarning) {
      if (occupancy < m_onlineOccError) {
        if (ocStatus < warning) ocStatus = warning;
      } else {
        if (ocStatus < error) ocStatus = error;
      }
    }
  } else {
    if (occupancy <= m_occEmpty) {
      if (ocStatus < lowStat) ocStatus = lowStat;
    } else if (occupancy > m_occWarning) {
      if (occupancy < m_occError) {
        if (ocStatus < warning) ocStatus = warning;
      } else {
        if (ocStatus < error) m_occUstatus = error;
      }
    }
  }
}
