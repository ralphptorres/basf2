/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdTimeGrouping/SVDTimeGroupingModule.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

#include <TH1D.h>
#include <TF1.h>
#include <TMath.h>

using namespace std;
using namespace Belle2;


REG_MODULE(SVDTimeGrouping);

double mygaus(const double* x, const double* par)
{
  return par[0] * exp(-0.5 * std::pow((x[0] - par[1]) / par[2], 2)) / (sqrt(2.*TMath::Pi()) * par[2]);
}

SVDTimeGroupingModule::SVDTimeGroupingModule() :
  Module()
{
  setDescription("Assigns the time-group Id to SVD clusters.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("SVDClusters", m_svdClustersName,
           "SVDCluster collection name", string(""));

  // 2. Fill time Histogram:
  addParam("tRangeLow", m_tRangeLow, "This sets the x- range of histogram [ns].",
           double(-160.));
  addParam("tRangeHigh", m_tRangeHigh, "This sets the x+ range of histogram [ns].",
           double(160.));
  addParam("rebinningFactor", m_rebinningFactor,
           "Time bin width is 1/rebinningFactor ns. Disables the module if zero",
           int(2));
  addParam("fillSigmaN", m_fillSigmaN,
           "Number of Gaussian sigmas (= hardcoded resolutions) used to fill the time histogram for each cluster.",
           double(3.));

  // 3. Search peaks:
  addParam("minSigma", m_minSigma,
           "Lower limit of cluster time sigma for the fit for the peak-search [ns].",
           double(1.));
  addParam("maxSigma", m_maxSigma,
           "Upper limit of cluster time sigma for the fit for the peak-search [ns].",
           double(15.));
  addParam("fitRangeHalfWidth", m_fitRangeHalfWidth,
           "half width of the range in which the fit for the peak-search is performed [ns].",
           double(5.));
  addParam("removeSigmaN", m_removeSigmaN,
           "Evaluate and remove gauss upto N sigma.",
           double(5.));
  addParam("fracThreshold", m_fracThreshold,
           "Minimum fraction of candidates in a peak considered for fitting for the peak-search.",
           double(0.05));
  addParam("maxGroups", m_maxGroups,
           "Maximum number of groups to be accepted.",
           int(20));

  // 4. Sort groups:
  addParam("expectedSignalTimeCenter", m_expectedSignalTimeCenter,
           "Expected time of the signal [ns].",
           double(0.));
  addParam("expectedSignalTimeMin", m_expectedSignalTimeMin,
           "Expected low range of signal hits [ns].",
           double(-50.));
  addParam("expectedSignalTimeMax", m_expectedSignalTimeMax,
           "Expected high range of signal hits [ns].",
           double(50.));
  addParam("signalLifetime", m_signalLifetime,
           "Group prominence is weighted with exponential weight with a lifetime defined by this parameter [ns].",
           double(30.));

  // 5. Signal group selection:
  addParam("numberOfSignalGroups", m_numberOfSignalGroups,
           "Number of groups expected to contain the signal clusters.",
           int(1));
  addParam("formSuperGroup", m_formSuperGroup,
           "Form a single super-group.",
           bool(false));
  addParam("acceptSigmaN", m_acceptSigmaN,
           "Accept clusters upto N sigma.",
           double(5.));
  addParam("writeGroupInfo", m_writeGroupInfo,
           "Write group info into SVDClusters.",
           bool(true));

  // 6. Hande out of range clusters:
  addParam("includeOutOfRangeClusters", m_includeOutOfRangeClusters,
           "Assign groups to under and overflow.",
           bool(true));

}



void SVDTimeGroupingModule::initialize()
{
  // prepare all store:
  m_svdClusters.isRequired(m_svdClustersName);

  if (m_numberOfSignalGroups != m_maxGroups) m_includeOutOfRangeClusters = false;

  if (m_rebinningFactor <= 0) B2WARNING("Module is ineffective.");
  if (m_tRangeHigh - m_tRangeLow < 10.) B2FATAL("tRange should not be less than 10 (hard-coded).");

  B2DEBUG(1, "SVDTimeGroupingModule \nsvdClusters: " << m_svdClusters.getName());
}



void SVDTimeGroupingModule::event()
{
  int totClusters = m_svdClusters.getEntries();
  if (m_rebinningFactor <= 0 || totClusters < 10) return;

  // number of clusters in signalRange
  double tmpRange[2] = {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()};
  for (int ij = 0; ij < totClusters; ij++) {
    double clsTime = m_svdClusters[ij]->getClsTime();
    if (std::isnan(tmpRange[0]) || clsTime > tmpRange[0]) tmpRange[0] = clsTime;
    if (std::isnan(tmpRange[1]) || clsTime < tmpRange[1]) tmpRange[1] = clsTime;
  }
  double tRangeHigh = m_tRangeHigh;
  double tRangeLow  = m_tRangeLow;
  if (tRangeHigh > tmpRange[0]) tRangeHigh = tmpRange[0];
  if (tRangeLow  < tmpRange[1]) tRangeLow  = tmpRange[1];

  int xbin = tRangeHigh - tRangeLow;
  if (xbin < 1) xbin = 1;
  xbin *= m_rebinningFactor;
  if (xbin < 2) xbin = 2;
  B2DEBUG(1, "tRange: [" << tRangeLow << "," << tRangeHigh << "], xBin: " << xbin);


  /** declaring histogram */
  TH1D h_clsTime = TH1D("h_clsTime", "h_clsTime", xbin, tRangeLow, tRangeHigh);
  for (int ij = 0; ij < totClusters; ij++) {
    float clsSize = m_svdClusters[ij]->getSize();
    bool  isUcls  = m_svdClusters[ij]->isUCluster();
    float gSigma  = (clsSize >= int(m_clsSizeVsSigma[isUcls].size()) ?
                     m_clsSizeVsSigma[isUcls].back() : m_clsSizeVsSigma[isUcls][clsSize - 1]);
    float gCenter = m_svdClusters[ij]->getClsTime();
    int  startBin = h_clsTime.FindBin(gCenter - m_fillSigmaN * gSigma);
    int    endBin = h_clsTime.FindBin(gCenter + m_fillSigmaN * gSigma);
    if (startBin < 1) startBin = 1;
    if (endBin > xbin) endBin = xbin;
    for (int ijx = startBin; ijx <= endBin; ijx++) {
      float tbinc = h_clsTime.GetBinCenter(ijx);
      h_clsTime.Fill(tbinc, TMath::Gaus(tbinc, gCenter, gSigma, true));
    }
  }

  std::vector<std::tuple<double, double, double>> groupInfo; // pars
  double maxPeak = 0.;
  double maxNorm = 0.;
  while (1) {

    int maxBin       = h_clsTime.GetMaximumBin();
    double maxBinPos = h_clsTime.GetBinCenter(maxBin);
    double maxBinCnt = h_clsTime.GetBinContent(maxBin);
    if (maxPeak == 0 && maxBinPos > m_expectedSignalTimeMin && maxBinPos < m_expectedSignalTimeMax)
      maxPeak = maxBinCnt;
    if (maxPeak != 0 && maxBinCnt < maxPeak * m_fracThreshold) break;

    TF1 ngaus("ngaus", mygaus, tRangeLow, tRangeHigh, 3);
    double maxPar0 = maxBinCnt * std::sqrt(2.*TMath::Pi()) * m_fitRangeHalfWidth;
    ngaus.SetParameter(0, maxBinCnt); ngaus.SetParLimits(0, maxPar0 * 0.01, maxPar0 * 2.);
    ngaus.SetParameter(1, maxBinPos);
    ngaus.SetParLimits(1, maxBinPos - m_fitRangeHalfWidth * 0.2, maxBinPos + m_fitRangeHalfWidth * 0.2);
    ngaus.SetParameter(2, m_fitRangeHalfWidth); ngaus.SetParLimits(2, m_minSigma, m_maxSigma);
    int status = h_clsTime.Fit("ngaus", "NQ0", "", maxBinPos - m_fitRangeHalfWidth, maxBinPos + m_fitRangeHalfWidth);
    if (!status) {
      double pars[3] = {ngaus.GetParameter(0), ngaus.GetParameter(1), std::fabs(ngaus.GetParameter(2))};
      if (pars[2] <= m_minSigma + 0.01) break;
      if (pars[2] >= m_maxSigma - 0.01) break;
      if (maxPeak != 0 && maxNorm == 0) maxNorm = pars[0];
      if (maxNorm != 0 && pars[0] < maxNorm * m_fracThreshold) break;

      int startBin = h_clsTime.FindBin(pars[1] - m_removeSigmaN * pars[2]);
      int   endBin = h_clsTime.FindBin(pars[1] + m_removeSigmaN * pars[2]);
      if (startBin < 1) startBin = 1;
      if (endBin > xbin)  endBin = xbin;
      for (int ijx = startBin; ijx <= endBin; ijx++) {
        float tbinc = h_clsTime.GetBinCenter(ijx);
        float tbincontent = h_clsTime.GetBinContent(ijx) - ngaus.Eval(tbinc);
        h_clsTime.SetBinContent(ijx, tbincontent);
      }

      // print
      groupInfo.push_back(std::make_tuple(pars[0], pars[1], pars[2]));
      B2DEBUG(1, " group " << int(groupInfo.size())
              << " pars[0] " << pars[0] << " pars[1] " << pars[1] << " pars[2] " << pars[2]);
      if (int(groupInfo.size()) >= m_maxGroups) break;
    } else break;

  }

  // resizing to max
  groupInfo.resize(m_maxGroups, std::make_tuple(0., 0., 0.));

  // sorting groups
  // possible signal first, then others
  std::tuple<double, double, double> key;
  for (int ij = int(groupInfo.size()) - 2; ij >= 0; ij--) {
    key = groupInfo[ij];
    float keynorm = std::get<0>(key);
    float keymean = std::get<1>(key);
    bool isKeySignal = true;
    if (keynorm != 0. && (keymean < m_expectedSignalTimeMin || keymean > m_expectedSignalTimeMax)) isKeySignal = false;
    if (isKeySignal) continue;
    int kj = ij + 1;
    while (1) {
      if (kj >= int(groupInfo.size())) break;
      float grnorm = std::get<0>(groupInfo[kj]);
      float grmean = std::get<1>(groupInfo[kj]);
      bool isGrSignal = true;
      if (grnorm != 0. && (grmean < m_expectedSignalTimeMin || grmean > m_expectedSignalTimeMax)) isGrSignal = false;
      if (!isGrSignal && (grnorm > keynorm)) break;
      groupInfo[kj - 1] = groupInfo[kj];
      kj++;
    }
    groupInfo[kj - 1] = key;
  }

  // sorting signal groups based on expo-weight
  // this decreases chance of near-signal bkg groups getting picked
  if (m_signalLifetime > 0.)
    for (int ij = 1; ij < int(groupInfo.size()); ij++) {
      key = groupInfo[ij];
      float keynorm = std::get<0>(key);
      if (keynorm <= 0) break;
      float keymean = std::get<1>(key);
      bool isKeySignal = true;
      if (keynorm > 0 && (keymean < m_expectedSignalTimeMin || keymean > m_expectedSignalTimeMax)) isKeySignal = false;
      if (!isKeySignal) break;
      float keyWt = keynorm * TMath::Exp(-std::fabs(keymean - m_expectedSignalTimeCenter) / m_signalLifetime);
      int kj = ij - 1;
      while (1) {
        if (kj < 0) break;
        float grnorm = std::get<0>(groupInfo[kj]);
        float grmean = std::get<1>(groupInfo[kj]);
        float grWt = grnorm * TMath::Exp(-std::fabs(grmean - m_expectedSignalTimeCenter) / m_signalLifetime);
        if (grWt > keyWt) break;
        groupInfo[kj + 1] = groupInfo[kj];
        kj--;
      }
      groupInfo[kj + 1] = key;
    }

  if (m_numberOfSignalGroups < int(groupInfo.size())) groupInfo.resize(m_numberOfSignalGroups);

  // make all clusters groupless if no groups are found
  if (int(groupInfo.size()) == 0)
    for (int jk = 0; jk < totClusters; jk++)
      m_svdClusters[jk]->setTimeGroupId().push_back(-1);

  for (int ij = 0; ij < int(groupInfo.size()); ij++) {
    double pars[3] = {std::get<0>(groupInfo[ij]), std::get<1>(groupInfo[ij]), std::get<2>(groupInfo[ij])};
    if (pars[2] == 0 && ij != int(groupInfo.size()) - 1) continue;
    double beginPos = pars[1] - m_acceptSigmaN * pars[2];
    double   endPos = pars[1] + m_acceptSigmaN * pars[2];
    if (beginPos < tRangeLow) beginPos = tRangeLow;
    if (endPos > tRangeHigh)  endPos   = tRangeHigh;
    B2DEBUG(1, " group " << ij
            << " beginPos " << beginPos << " endPos " << endPos);
    for (int jk = 0; jk < totClusters; jk++) {
      double clsTime = m_svdClusters[jk]->getClsTime();
      if (pars[2] != 0 && clsTime >= beginPos && clsTime <= endPos) {
        if (m_formSuperGroup) {
          if (int(m_svdClusters[jk]->getTimeGroupId().size()) == 0)
            m_svdClusters[jk]->setTimeGroupId().push_back(0);
        } else
          m_svdClusters[jk]->setTimeGroupId().push_back(ij);
        if (m_writeGroupInfo) m_svdClusters[jk]->setTimeGroupInfo().push_back(std::make_tuple(pars[0], pars[1], pars[2]));
        B2DEBUG(1, "   accepted cluster " << jk
                << " clsTime " << clsTime
                << " GroupId " << m_svdClusters[jk]->getTimeGroupId().back());
      } else {
        B2DEBUG(1, "     rejected cluster " << jk
                << " clsTime " << clsTime);
        if (ij == int(groupInfo.size()) - 1 && int(m_svdClusters[jk]->getTimeGroupId().size()) == 0) { // leftover clusters
          if (m_includeOutOfRangeClusters &&
              clsTime < tRangeLow)
            m_svdClusters[jk]->setTimeGroupId().push_back(ij + 1);       // underflow
          else if (m_includeOutOfRangeClusters &&
                   clsTime > tRangeHigh)
            m_svdClusters[jk]->setTimeGroupId().push_back(ij + 2);       // overflow
          else
            m_svdClusters[jk]->setTimeGroupId().push_back(-1);           // orphan
          B2DEBUG(1, "     leftover cluster " << jk
                  << " GroupId " << m_svdClusters[jk]->getTimeGroupId().back());
        }
      }
    }
  }

}

