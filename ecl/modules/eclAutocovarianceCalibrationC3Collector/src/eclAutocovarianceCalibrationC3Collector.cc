/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//This module`
#include <ecl/modules/eclAutocovarianceCalibrationC3Collector/eclAutocovarianceCalibrationC3Collector.h>

#include <iostream>
//Framework
#include <framework/dataobjects/EventMetaData.h>

//ECL
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dbobjects/ECLCrystalCalib.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Modules
//-----------------------------------------------------------------
REG_MODULE(eclAutocovarianceCalibrationC3Collector)
//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

// constructor
eclAutocovarianceCalibrationC3CollectorModule::eclAutocovarianceCalibrationC3CollectorModule() : CalibrationCollectorModule(),
  m_ECLAutocovarianceCalibrationC1Threshold("ECLAutocovarianceCalibrationC1Threshold"),
  m_ECLAutocovarianceCalibrationC2Baseline("ECLAutocovarianceCalibrationC2Baseline")
{
  // Set module properties
  setDescription("Module to export histogram of noise in waveforms from random trigger events");
  setPropertyFlags(c_ParallelProcessingCertified);
}

void eclAutocovarianceCalibrationC3CollectorModule::prepare()
{

  count = 0;

  /**----------------------------------------------------------------------------------------*/
  B2INFO("eclAutocovarianceCalibrationC3Collector: Experiment = " << m_evtMetaData->getExperiment() << "  run = " <<
         m_evtMetaData->getRun());

  /**----------------------------------------------------------------------------------------*/
  /** Create the histograms and register them in the data store */
  CovarianceMatrixInfoVsCrysID = new TH2F("CovarianceMatrixInfoVsCrysID", "", 8736, 0, 8736, 32, 0, 32);
  registerObject<TH2F>("CovarianceMatrixInfoVsCrysID", CovarianceMatrixInfoVsCrysID);

  m_PeakToPeakThresholds = m_ECLAutocovarianceCalibrationC1Threshold->getCalibVector();

  m_Baselines = m_ECLAutocovarianceCalibrationC2Baseline->getCalibVector();

  m_eclDsps.registerInDataStore();
  m_eclDigits.registerInDataStore();

  for (int i = 0; i < 8736; i++) {

    for (int j = 0; j < 32; j++) myHist[i][j] = 0.0;

  }
}


void eclAutocovarianceCalibrationC3CollectorModule::collect()
{

  const int NumDsp = m_eclDsps.getEntries();

  //Random Trigger Event
  //if (NumDsp == 8736 and count<100) {
  if (NumDsp == 8736) {

    for (auto& aECLDsp : m_eclDsps) {

      const int id = aECLDsp.getCellId() - 1;

      int minADC = aECLDsp.getDspA()[0];
      int maxADC = minADC;

      for (int i = 1; i < 31; i++) {

        int value = aECLDsp.getDspA()[i];
        if (value < minADC) minADC = value;
        if (value > maxADC) maxADC = value;

      }

      int PeakToPeak = maxADC - minADC;


      if (PeakToPeak < m_PeakToPeakThresholds[id]) {

        float baseline = m_Baselines[id];

        m_tempArray.clear();
        for (int i = 0; i < 31; i++) m_tempArray.push_back(aECLDsp.getDspA()[i] - baseline);

        for (int i = 0; i < 31; i++) {

          float value_i = m_tempArray[i];

          for (int j = i; j < 31; j++) {

            int tempIndex = abs(i - j);

            myHist[id][tempIndex] += (value_i * m_tempArray[j]);

          }
        }
        myHist[id][31]++;
      }
    }
  }

  count++;
  if (count % 10 == 0) std::cout << count << std::endl;;
}

void eclAutocovarianceCalibrationC3CollectorModule::closeRun()
{
  for (int i = 0; i < 8736; i++) {
    for (int j = 0; j < 32; j++) {
      getObjectPtr<TH2>("CovarianceMatrixInfoVsCrysID")->SetBinContent(i + 1, j + 1, myHist[i][j]);
    }
  }
}
