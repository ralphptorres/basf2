/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once
#include <vector>
#include <utility>
#include <tuple>
#include <TVector3.h>
#include <Eigen/Dense>

//If compiled within BASF2
#ifdef _PACKAGE_
#include <tracking/calibration/Splitter.h>
#else
#include <Splitter.h>
#endif

namespace Belle2 {
  namespace InvariantMassBhadCalib {

    static const double realNaN = std::numeric_limits<double>::quiet_NaN();
    static const int intNaN     = std::numeric_limits<int>::quiet_NaN();
    static const TVector3 vecNaN(realNaN, realNaN, realNaN);


    /** structure containing variables relevant for the hadronic B decays */
    struct Event {

      int exp   = intNaN;  ///< experiment number
      int run   = intNaN;  ///< run number
      int evtNo = intNaN;  ///< event number

      double mBC = realNaN;     ///< beam-constrained mass of B meson
      double deltaE = realNaN;  ///< eBmeson - eCMS/2 in the centre-of-mass frame
      int pdg = intNaN;         ///< PDG code of the signal B-meson
      int mode = intNaN;        ///< integer code identifying the decay channel
      double Kpid = realNaN;    ///< Kaon PID
      double R2 = realNaN;      ///< R2 continuous suppression variable
      double mD = realNaN;      ///< reconstructed mass of the D meson
      double dmDstar = realNaN; ///< reconstructed mass difference between Dstar and D
      //double cmsE0; // eCMS used to calculate mBC and deltaE

      double t = realNaN;       ///< time of the event

      bool isSig = false;       ///< isSignal flag (for applying selections)
      int nBootStrap = intNaN;  ///< bootstap weight
    };






    std::vector<Event> getEvents(TTree* tr);


    std::vector<std::vector<double>> doBhadFit(const std::vector<Event>& evts, std::vector<std::pair<double, double>> limits,
                                               std::vector<std::pair<double, double>> mumuVals);



    /** Run the InvariantMass analysis where splitPoints are the boundaries of the short calibration intervals
      @param evts: vector of events
      @param splitPoints: the vector containing times of the edges of the short calibration intervals [hours]
      @return A tuple containing vector with invariant mass, vector with invariant mass stat. error and a spread of the invariant mass
    */
    std::tuple<std::vector<Eigen::VectorXd>, std::vector<Eigen::MatrixXd>, Eigen::MatrixXd>  runInvariantMassAnalysis(
      std::vector<Event> evts,
      const std::vector<double>& splitPoints);

  }
}
