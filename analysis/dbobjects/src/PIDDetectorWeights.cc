/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/gearbox/Unit.h>
#include <analysis/dbobjects/PIDDetectorWeights.h>

using namespace Belle2;


void PIDDetectorWeights::fillWeightsTablePerHypoFromRDF()
{

  auto dummyBinEdges = std::set<double>();

  // Flatten the RDataFrame content into the tabular structure for each std charged hypothesis.
  for (const auto& hypo : Const::chargedStableSet) {

    WeightsTable weightsTable;

    auto cut_pdgId = [&](double pdg) { return (pdg == hypo.getPDGCode()); };
    auto filteredRDF = m_weightsRDataFrame.Filter(cut_pdgId, {"pdgId"});

    if (*filteredRDF.Count()) {

      // Get the lower and upper bin edges from the RDF as C++ vectors
      auto pMinEdges = filteredRDF.Take<double>("p_min").GetValue();
      auto pMaxEdges = filteredRDF.Take<double>("p_max").GetValue();
      // Get the bin indexes.
      auto pBinIdxs = filteredRDF.Take<double>("p_bin_idx").GetValue();
      // Convert to a set and take the union
      std::set<double> pMinEdges_set(std::begin(pMinEdges), std::end(pMinEdges));
      std::set<double> pMaxEdges_set(std::begin(pMaxEdges), std::end(pMaxEdges));
      std::set<double> pBinEdges;
      std::set_union(std::begin(pMinEdges_set), std::end(pMinEdges_set),
                     std::begin(pMaxEdges_set), std::end(pMaxEdges_set),
                     std::inserter(pBinEdges, std::begin(pBinEdges)));
      // Store the set of bin edges into the internal tabular structure for this particle hypothesis.
      weightsTable.m_pBinEdges = pBinEdges;

      // Store the nr. of p bins. Used for bin idx linearisation.
      weightsTable.m_nPBins = pBinEdges.size() - 1;

      auto thetaMinEdges = filteredRDF.Take<double>("theta_min").GetValue();
      auto thetaMaxEdges = filteredRDF.Take<double>("theta_max").GetValue();
      auto thetaBinIdxs = filteredRDF.Take<double>("theta_bin_idx").GetValue();
      std::set<double> thetaMinEdges_set(std::begin(thetaMinEdges), std::end(thetaMinEdges));
      std::set<double> thetaMaxEdges_set(std::begin(thetaMaxEdges), std::end(thetaMaxEdges));
      std::set<double> thetaBinEdges;
      std::set_union(std::begin(thetaMinEdges_set), std::end(thetaMinEdges_set),
                     std::begin(thetaMaxEdges_set), std::end(thetaMaxEdges_set),
                     std::inserter(thetaBinEdges, std::begin(thetaBinEdges)));
      weightsTable.m_thetaBinEdges = thetaBinEdges;
      weightsTable.m_nThetaBins = thetaBinEdges.size() - 1;

      // Get the p, theta bin index columns and zip them together.
      std::vector<std::tuple<double, double>> pAndThetaIdxs;
      std::transform(std::begin(pBinIdxs), std::end(pBinIdxs),
                     std::begin(thetaBinIdxs),
      std::back_inserter(pAndThetaIdxs), [](const auto & pIdx, const auto & thetaIdx) { return std::make_tuple(pIdx, thetaIdx); });
      // And now, fill a unordered_map with the linearised (p, theta) bin index as key and the vector index as value for fast lookup.
      unsigned int iRow(0);
      for (const auto& tup : pAndThetaIdxs) {
        double linBinIdx = (std::get<0>(tup) - 1.0) + (std::get<1>(tup) - 1.0) * weightsTable.m_nPBins;
        weightsTable.m_linBinIdxsToRowIdxs.insert(std::make_pair(linBinIdx, iRow));
        ++iRow;
      }

      // Store the vector (column) of weights per detector.
      for (unsigned int iDet(0); iDet < Const::PIDDetectors::c_size; iDet++) {
        const auto det =  Const::PIDDetectors::c_set[iDet];
        auto detName = Const::parseDetectors(det);
        auto colName = "ablat_s_" + detName;
        auto weights = filteredRDF.Take<double>(colName.c_str()).GetValue();
        weightsTable.m_weightsPerDet.insert(std::make_pair(detName, weights));
      }

    } else {

      B2WARNING("Couldn't find detector weights in input ROOT::RDataFrame for std charged particle hypothesis: " << hypo.getPDGCode());
      weightsTable.m_isEmpty = true;

    }

    m_weightsTablePerHypo.insert(std::make_pair(hypo.getPDGCode(), weightsTable));

  };

}


double PIDDetectorWeights::getWeight(Const::ChargedStable hypo, Const::EDetector det, const double p, const double theta)
{

  const auto weightsTable = &m_weightsTablePerHypo[hypo.getPDGCode()];

  if (weightsTable->m_isEmpty) {
    // Weights not available for this pdgId.
    return std::numeric_limits<float>::quiet_NaN();
  }

  // Get the bin index of the input p value.
  auto pBinUpperEdge_it = std::lower_bound(std::begin(weightsTable->m_pBinEdges),
                                           std::end(weightsTable->m_pBinEdges),
                                           p);
  auto pBinIdx = std::distance(std::begin(weightsTable->m_pBinEdges), pBinUpperEdge_it);
  // Get the bin index of the input theta value.
  auto thetaBinUpperEdge_it = std::lower_bound(std::begin(weightsTable->m_thetaBinEdges),
                                               std::end(weightsTable->m_thetaBinEdges),
                                               theta);
  auto thetaBinIdx = std::distance(std::begin(weightsTable->m_thetaBinEdges), thetaBinUpperEdge_it);

  // Get the linearised bin index to look up.
  double linBinIdx = (pBinIdx - 1.0) + (thetaBinIdx - 1.0) * weightsTable->m_nPBins;

  if (!weightsTable->m_linBinIdxsToRowIdxs.count(linBinIdx)) {
    // Out-of-bin range p or theta
    B2WARNING("p = " << p
              << " [GeV/c], theta = " << theta
              << " [rad] - Either is outside of bin range. Bin indexes: ("
              <<  pBinIdx << ", " << thetaBinIdx << ").");
    return std::numeric_limits<float>::quiet_NaN();
  }

  auto rowIdx = weightsTable->m_linBinIdxsToRowIdxs[linBinIdx];

  return weightsTable->m_weightsPerDet[Const::parseDetectors(det)].at(rowIdx);

};
