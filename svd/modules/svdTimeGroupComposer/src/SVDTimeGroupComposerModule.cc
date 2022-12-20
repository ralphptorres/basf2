/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <svd/modules/svdTimeGroupComposer/SVDTimeGroupComposerModule.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

using namespace std;
using namespace Belle2;


REG_MODULE(SVDTimeGroupComposer);

SVDTimeGroupComposerModule::SVDTimeGroupComposerModule() :
  Module()
{
  setDescription("Imports Clusters of the SVD detector and Assign time-group Id.");
  setPropertyFlags(c_ParallelProcessingCertified);

  // 1. Collections.
  addParam("SVDClusters", m_svdClustersName,
           "SVDCluster collection name", string(""));
  addParam("EventLevelTrackingInfoName", m_eventLevelTrackingInfoName,
           "EventLevelTrackingInfo collection name", string(""));

  // 2.Modification parameters:
  addParam("useOnlyOneGroup", m_useOnlyOneGroup,
           "Clusters belonging to the group nearest to zero is kept.",
           bool(false));
  addParam("averageCountPerBin", m_AverageCountPerBin,
           "This sets the bin width of histogram. Setting it zero or less disables the module.",
           double(1.));
  addParam("xRange", m_xRange, "This sets the x-range of histogram in ns.",
           double(160.));
  addParam("threshold", m_threshold, "Bin Content bellow this is not considered.",
           double(1.));
}



void SVDTimeGroupComposerModule::initialize()
{
  // prepare all store:
  m_svdClusters.isRequired(m_svdClustersName);

  if (m_useOnlyOneGroup) B2WARNING("Only the group nearest to zero is selected.");
  if (m_AverageCountPerBin <= 0.) B2WARNING("averageCountPerBin is set to zero or less."
                                              << " Module is ineffective.");
  if (m_xRange < 10.)             B2FATAL("xRange should not be less than 10 (hard-coded).");

  B2DEBUG(1, "SVDTimeGroupComposerModule \nsvdClusters: " << m_svdClusters.getName());
  B2INFO("SVDTimeGroupComposer : AverageCountPerBin = " << m_AverageCountPerBin);
  B2INFO("SVDTimeGroupComposer : Xrange             = " << m_xRange);
  B2INFO("SVDTimeGroupComposer : Threshold          = " << m_threshold);
}



void SVDTimeGroupComposerModule::event()
{
  int totClusters = m_svdClusters.getEntries();
  int xbin = totClusters / (m_AverageCountPerBin > 0 ? m_AverageCountPerBin : 1.); // safety
  if (m_AverageCountPerBin > 0 && xbin > 0) {
    /** declaring histogram */
    TH1D h_clsTime("h_clsTime", "h_clsTime", xbin, -m_xRange, m_xRange);
    for (int ij = 0; ij < totClusters; ij++) {
      h_clsTime.Fill(m_svdClusters[ij]->getClsTime());
    }

    /** finalized the groups */
    int groupBegin = -1; int groupEnd = -1;
    std::vector<std::tuple<double, double, int>> groupInfo; // start, end, totCls
    for (int ij = 1; ij <= xbin; ij++) {
      double sumc = h_clsTime.GetBinContent(ij);
      double suml = (ij == 1    ? 0. : h_clsTime.GetBinContent(ij - 1));
      double sumr = (ij == xbin ? 0. : h_clsTime.GetBinContent(ij + 1));
      // possible background
      if (sumc <= m_threshold) sumc = 0;
      if (suml <= m_threshold) suml = 0;
      if (sumr <= m_threshold) sumr = 0;

      double sum = suml + sumc + sumr;
      // finding group
      if (sum > 0 && groupBegin < 0 && groupEnd < 0) { groupBegin = ij;}
      if (sum <= 0 && groupBegin > 0 && groupEnd < 0) {
        groupEnd = ij - 1;
        int clsInGroup = h_clsTime.Integral(groupBegin, groupEnd);
        double beginPos = h_clsTime.GetXaxis()->GetBinLowEdge(groupBegin);
        double endPos   = h_clsTime.GetXaxis()->GetBinLowEdge(groupEnd) +
                          h_clsTime.GetXaxis()->GetBinWidth(groupEnd);
        if (clsInGroup > 1) {
          groupInfo.push_back(std::make_tuple(beginPos, endPos, clsInGroup));
          B2DEBUG(1, " group " << ij
                  << " beginPos " << beginPos << " endPos " << endPos
                  << " totCls " << clsInGroup);
        }
        groupBegin = groupEnd = -1; // reset for new group
      }
    }
    double underflow = h_clsTime.GetBinContent(0);
    double overflow  = h_clsTime.GetBinContent(xbin + 1);

    // sorting groups in descending cluster-counts or distance from zero
    // this should help speed up the next process
    std::tuple<double, double, int> key;
    for (int ij = 1; ij < int(groupInfo.size()); ij++) {
      key = groupInfo[ij];
      int kj = ij - 1;
      if (!m_useOnlyOneGroup) // sort in cluster time
        while ((kj >= 0) &&
               ((std::get<2>(groupInfo[kj])) < (std::get<2>(key)))) {
          groupInfo[kj + 1] = groupInfo[kj];
          kj--;
        } else    // sort in distance from zero
        while ((kj >= 0) &&
               std::min(std::fabs(std::get<0>(groupInfo[kj])), std::fabs(std::get<1>(groupInfo[kj]))) > std::min(std::fabs(std::get<0>(key)),
                   std::fabs(std::get<1>(key)))) {
          groupInfo[kj + 1] = groupInfo[kj];
          kj--;
        }
      groupInfo[kj + 1] = key;
    }

    if (m_useOnlyOneGroup && int(groupInfo.size())) // keep only one group
      groupInfo.resize(1);

    std::vector<int> rejectedCls;
    rejectedCls.assign(totClusters, -1);
    int totGroups = int(groupInfo.size());
    for (int ij = 0; ij < totGroups; ij++) {
      auto beginPos    = std::get<0>(groupInfo[ij]);
      auto endPos      = std::get<1>(groupInfo[ij]);
      auto totCls      = std::get<2>(groupInfo[ij]);

      B2DEBUG(1, " group " << ij
              << " beginPos " << beginPos << " endPos " << endPos
              << " totCls " << totCls);
      int rejectedCount = 0;
      for (int jk = 0; jk < int(rejectedCls.size()); jk++) {
        int place = rejectedCls[jk] < 0 ? jk : rejectedCls[jk];
        double clsTime = m_svdClusters[place]->getClsTime();
        if (clsTime >= beginPos && clsTime <= endPos) {
          m_svdClusters[place]->setTimeGroupId(ij);
          B2DEBUG(1, "   accepted cluster " << place
                  << " clsTime " << clsTime);
        } else {
          B2DEBUG(1, "     rejected cluster " << place
                  << " clsTime " << clsTime);
          if (ij == totGroups - 1) {                              // leftover clusters
            if (!m_useOnlyOneGroup &&
                clsTime < -m_xRange && underflow > m_threshold)
              m_svdClusters[place]->setTimeGroupId(ij + 1);       // underflow
            else if (!m_useOnlyOneGroup &&
                     clsTime > m_xRange && overflow > m_threshold)
              m_svdClusters[place]->setTimeGroupId(ij + 2);       // overflow
            else
              m_svdClusters[place]->setTimeGroupId(-1);           // orphan
            B2DEBUG(1, "     leftover cluster " << place
                    << " GroupId " << m_svdClusters[place]->getTimeGroupId());
          } else {
            rejectedCls[rejectedCount++] = place;
          }
        }
      } // for(int jk=0;jk<int(rejectedCls.size());jk++) {
      rejectedCls.resize(rejectedCount);
    } // for(int ij=0;ij<int(groupInfo.size());ij++) {
  }   // if(totClusters > 2) {
}

