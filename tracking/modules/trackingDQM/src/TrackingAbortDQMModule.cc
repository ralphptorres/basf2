/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "tracking/modules/trackingDQM/TrackingAbortDQMModule.h"

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <TDirectory.h>
#include <TLine.h>
#include <TStyle.h>

#include <algorithm>
#include <string>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TrackingAbortDQM);


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackingAbortDQMModule::TrackingAbortDQMModule() : HistoModule()
{
  setDescription("DQM Module to monitor Tracking-related quantities before the HLT filter.");

  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed.",
           std::string("TrackingAbort"));

  setPropertyFlags(c_ParallelProcessingCertified);
}


TrackingAbortDQMModule::~TrackingAbortDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void TrackingAbortDQMModule::defineHisto()
{

  // Create a separate histogram directories and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());
    oldDir->cd(m_histogramDirectoryName.c_str());
  }
  //histogram index:
  // 0 if the event is triggered OUTSIDE the active_veto window
  string tag[2] = {"OUT", "IN"};
  string title[2] = {"[Outside Active Veto Window]", "[Inside Active Veto Window]"};


  //number of events with and without at least one abort
  //outside active_veto window:
  string histoName = "EventsWithAborts";
  string histoTitle = "Events With at Least one Abort";
  m_nEventsWithAbort[0] = new TH1F(TString::Format("%s_%s", histoName.c_str(), tag[0].c_str()),
                                   TString::Format("%s %s", histoTitle.c_str(), title[0].c_str()),
                                   2, -0.5, 1.5);
  m_nEventsWithAbort[0]->GetXaxis()->SetBinLabel(1, "No Abort");
  m_nEventsWithAbort[0]->GetXaxis()->SetBinLabel(2, "At Least One Abort");
  m_nEventsWithAbort[0]->SetMinimum(0.1);

  //inside active_veto window:
  m_nEventsWithAbort[1] = new TH1F(*m_nEventsWithAbort[0]);
  m_nEventsWithAbort[1]->SetName(TString::Format("%s_%s", histoName.c_str(), tag[1].c_str()));
  m_nEventsWithAbort[1]->SetTitle(TString::Format("%s %s", histoTitle.c_str(), title[1].c_str()));

  //abort flag reason
  //outside active_veto window:
  histoName = "TrkAbortReason";
  histoTitle = "Tracking Abort Reason";
  m_trackingErrorFlagsReasons[0] = new TH1F(TString::Format("%s_%s", histoName.c_str(), tag[0].c_str()),
                                            TString::Format("%s %s", histoTitle.c_str(), title[0].c_str()),
                                            5, -0.5, 4.5);
  m_trackingErrorFlagsReasons[0]->GetXaxis()->SetTitle("Type of error occurred");
  m_trackingErrorFlagsReasons[0]->GetYaxis()->SetTitle("Number of events");
  m_trackingErrorFlagsReasons[0]->GetXaxis()->SetBinLabel(1, "Unspecified PR");
  m_trackingErrorFlagsReasons[0]->GetXaxis()->SetBinLabel(2, "VXDTF2");
  m_trackingErrorFlagsReasons[0]->GetXaxis()->SetBinLabel(3, "SVDCKF");
  m_trackingErrorFlagsReasons[0]->GetXaxis()->SetBinLabel(4, "PXDCKF");
  m_trackingErrorFlagsReasons[0]->GetXaxis()->SetBinLabel(5, "SpacePoint");
  //inside active_veto window:
  m_trackingErrorFlagsReasons[1] = new TH1F(*m_trackingErrorFlagsReasons[0]);
  m_trackingErrorFlagsReasons[1]->SetName(TString::Format("%s_%s", histoName.c_str(), tag[1].c_str()));
  m_trackingErrorFlagsReasons[1]->SetTitle(TString::Format("%s %s", histoTitle.c_str(), title[1].c_str()));


  //SVD L3 occupancy - see SVDDQMDose module for details
  histoName = "SVDL3VOcc";
  histoTitle = "SVD L3 v-side ZS5 Occupancy (%)";
  //outside active_veto window:
  m_svdL3vZS5Occupancy[0] = new TH1F(TString::Format("%s_%s", histoName.c_str(), tag[0].c_str()),
                                     TString::Format("%s %s", histoTitle.c_str(), title[0].c_str()),
                                     90, 0, 100.0 / 1536.0 * 90);
  m_svdL3vZS5Occupancy[0]->GetXaxis()->SetTitle("occupancy [%]");
  m_svdL3vZS5Occupancy[0]->GetYaxis()->SetTitle("Number Of Events");
  //inside active_veto window:
  m_svdL3vZS5Occupancy[1] = new TH1F(*m_svdL3vZS5Occupancy[0]);
  m_svdL3vZS5Occupancy[1]->SetName(TString::Format("%s_%s", histoName.c_str(), tag[1].c_str()));
  m_svdL3vZS5Occupancy[1]->SetTitle(TString::Format("%s %s", histoTitle.c_str(), title[1].c_str()));


  //CDC extra hits
  histoName = "nCDCExtraHits";
  histoTitle = "Number of CDC Extra Hits";
  //outside active_veto window:
  m_nCDCExtraHits[0] = new TH1F(TString::Format("%s_%s", histoName.c_str(), tag[0].c_str()),
                                TString::Format("%s %s", histoTitle.c_str(), title[0].c_str()),
                                200, 0, 5000);
  m_nCDCExtraHits[0]->GetXaxis()->SetTitle("nCDCExtraHits");
  m_nCDCExtraHits[0]->GetYaxis()->SetTitle("Number of Events");
  //inside active_veto window:
  m_nCDCExtraHits[1] = new TH1F(*m_nCDCExtraHits[0]);
  m_nCDCExtraHits[1]->SetName(TString::Format("%s_%s", histoName.c_str(), tag[1].c_str()));
  m_nCDCExtraHits[1]->SetTitle(TString::Format("%s %s", histoTitle.c_str(), title[1].c_str()));



  oldDir->cd();
}

void TrackingAbortDQMModule::initialize()
{
  m_eventLevelTrackingInfo.isOptional();
  m_eventMetaData.isOptional();

  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM
}


void TrackingAbortDQMModule::beginRun()
{

  if (m_trackingErrorFlagsReasons[0] != nullptr) m_trackingErrorFlagsReasons[0]->Reset();
  if (m_trackingErrorFlagsReasons[1] != nullptr) m_trackingErrorFlagsReasons[1]->Reset();
  if (m_nEventsWithAbort[0] != nullptr)  m_nEventsWithAbort[0]->Reset();
  if (m_nEventsWithAbort[1] != nullptr)  m_nEventsWithAbort[1]->Reset();
  if (m_svdL3vZS5Occupancy[0] != nullptr)  m_svdL3vZS5Occupancy[0]->Reset();
  if (m_svdL3vZS5Occupancy[1] != nullptr)  m_svdL3vZS5Occupancy[1]->Reset();
  if (m_nCDCExtraHits[0] != nullptr) m_nCDCExtraHits[0]->Reset();
  if (m_nCDCExtraHits[1] != nullptr) m_nCDCExtraHits[1]->Reset();
}


void TrackingAbortDQMModule::event()
{
  //skip the empty events
  bool eventIsEmpty = false;

  if (m_eventMetaData->getErrorFlag() & EventMetaData::EventErrorFlag::c_B2LinkPacketCRCError)
    eventIsEmpty = true;

  if (m_eventMetaData->getErrorFlag() & EventMetaData::EventErrorFlag::c_B2LinkEventCRCError)
    eventIsEmpty = true;

  if (m_eventMetaData->getErrorFlag() & EventMetaData::EventErrorFlag::c_HLTCrash)
    eventIsEmpty = true;

  if (m_eventMetaData->getErrorFlag() & EventMetaData::EventErrorFlag::c_ReconstructionAbort)
    eventIsEmpty = true;

  if (eventIsEmpty) return;


  //find out if we are in the passive veto (i=0) or in the active veto window (i=1)
  int index = 0; //events accepted in the passive veto window but not in the active

  int trgBit_rejectedBy_AV = m_trgSummary->getInputBitNumber("cdcecl_veto"); // 53
  int trgBit_rejectedBy_PV = m_trgSummary->getInputBitNumber("passive_veto"); // 28
  if (m_trgSummary->testInput(trgBit_rejectedBy_PV) == 1 &&  m_trgSummary->testInput(trgBit_rejectedBy_AV) == 0) index = 1;


  //fill the tracking abort reason histogram & nEvents with Abort
  if (m_eventLevelTrackingInfo.isValid()) {
    if (m_eventLevelTrackingInfo->hasAnErrorFlag()) {

      m_nEventsWithAbort[index]->Fill(1);

      if (m_eventLevelTrackingInfo->hasUnspecifiedTrackFindingFailure())
        m_trackingErrorFlagsReasons[index]->Fill(0);
      if (m_eventLevelTrackingInfo->hasVXDTF2AbortionFlag())
        m_trackingErrorFlagsReasons[index]->Fill(1);
      if (m_eventLevelTrackingInfo->hasSVDCKFAbortionFlag())
        m_trackingErrorFlagsReasons[index]->Fill(2);
      if (m_eventLevelTrackingInfo->hasPXDCKFAbortionFlag())
        m_trackingErrorFlagsReasons[index]->Fill(3);
      if (m_eventLevelTrackingInfo->hasSVDSpacePointCreatorAbortionFlag())
        m_trackingErrorFlagsReasons[index]->Fill(4);
    } else { //EventLevelTrackingIinfo valid but no error
      m_nEventsWithAbort[index]->Fill(0);
    }
  } else //EventLevelTrackingIinfo not valid
    m_nEventsWithAbort[index]->Fill(0);


  // fill the svd L3 v ZS5 occupancy
  float nStripsL3VZS5 = 0;
  for (const SVDShaperDigit& hit : m_strips) {
    const VxdID& sensorID = hit.getSensorID();
    if (sensorID.getLayerNumber() != 3) continue;
    if (hit.isUStrip()) continue;
    float noise = m_NoiseCal.getNoise(sensorID, 0, hit.getCellID());
    float cutMinSignal = 5 * noise + 0.5;
    cutMinSignal = (int)cutMinSignal;

    if (hit.passesZS(1, cutMinSignal)) nStripsL3VZS5++;
  }
  m_svdL3vZS5Occupancy[index]->Fill(std::min((double)nStripsL3VZS5 / nStripsL3V * 100, (double)5.82));

  //fill the nCDCExtraHits
  m_nCDCExtraHits[index]->Fill(std::min((int)m_eventLevelTrackingInfo->getNCDCHitsNotAssigned(), (int)4999));



}

