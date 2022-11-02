/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <trg/cdc/NeuroTrigger.h>
#include <trg/cdc/NeuroTriggerParameters.h>
#include <trg/cdc/dataobjects/CDCTriggerMLP.h>
//#include <trg/cdc/dbobjects/CDCTriggerNeuroConfig.h>
//#include <framework/database/DBObjPtr.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

//std::vector<float> loadweights (std::string filename) {
//    std::ifstream netfile(filename, std::ifstream::binary);
//    nlohmann::json nets;
//    netfile >> nets;
//    std::cout << nets["model.net.0.weight"][0] << std::endl;
//    std::vector<float> ret;
//    ret.push_back(4.);
//    return ret;
//
//
//
//}
using namespace Belle2;

int main(int argc, char* argv[])
{

  // get arguments
  if (argc < 4) {
    std::cout << "Program needs at 3 arguments:" << std::endl
              << " 1: json weights" << std::endl
              << " 2: configuration file name" << std::endl
              << " 3: output filename" << std::endl;
    return -1;
  }
  std::string cfn = argv[2];
  NeuroTriggerParameters p(cfn);


  NeuroTrigger m_nnt;
  m_nnt.initialize(p);
  m_nnt.loadIDHist("IDHist.gz");
  //if (!m_nnt.load(argv[1], "MLPs")) {
  //    std::cout << "Error loading file: " << argv[1] << std::endl;
  //}
  std::ifstream netfile(argv[1], std::ifstream::binary);
  nlohmann::json nets;
  netfile >> nets;

  for (unsigned expert = 0; expert < m_nnt.nSectors(); expert++) {

    std::vector<float> weights;
    int numnode = 0;
    for (auto node : nets["expert_" + std::to_string(expert)]["weights"]["model.net.0.weight"]) {
      for (float w : node) {
        weights.push_back(w);
      }
      weights.push_back(nets["expert_" + std::to_string(expert)]["weights"]["model.net.0.bias"][numnode]);
      ++numnode;
    }
    numnode = 0;
    for (auto node : nets["expert_" + std::to_string(expert)]["weights"]["model.net.2.weight"]) {
      for (float w : node) {
        weights.push_back(w);
      }
      weights.push_back(nets["expert_" + std::to_string(expert)]["weights"]["model.net.2.bias"][numnode]);
      ++numnode;
    }
    std::cout << " writing " << weights.size() << " weights for expert " << expert << std::endl;
    m_nnt[expert].setWeights(weights);
  }
  m_nnt.save(argv[3], "MLPs");


  return 0;
}
