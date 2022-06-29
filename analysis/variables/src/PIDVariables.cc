/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/variables/PIDVariables.h>

#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/PIDLikelihood.h>

// framework aux
#include <framework/logging/Logger.h>
#include <framework/utilities/Conversion.h>
#include <framework/gearbox/Const.h>

#include <boost/algorithm/string.hpp>

#include <iostream>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace Variable {



    //*************
    // Utilities
    //*************

    // converts Belle numbering scheme for charged final state particles
    // to Belle II ChargedStable
    Const::ChargedStable hypothesisConversion(const int hypothesis)
    {
      switch (hypothesis) {
        case 0:
          return Const::electron;
        case 1:
          return Const::muon;
        case 2:
          return Const::pion;
        case 3:
          return Const::kaon;
        case 4:
          return Const::proton;
      }

      return Const::pion;
    }


    Const::PIDDetectorSet parseDetectors(const std::vector<std::string>& arguments)
    {
      Const::PIDDetectorSet result;
      for (std::string val : arguments) {
        boost::to_lower(val);
        if (val == "all") return Const::PIDDetectors::set();
        else if (val == "svd") result += Const::SVD;
        else if (val == "cdc") result += Const::CDC;
        else if (val == "top") result += Const::TOP;
        else if (val == "arich") result += Const::ARICH;
        else if (val == "ecl") result += Const::ECL;
        else if (val == "klm") result += Const::KLM;
        else B2ERROR("Unknown detector component: " << val);
      }
      return result;
    }

    // Specialisation of valid detectors parser for ChargedBDT.
    Const::PIDDetectorSet parseDetectorsChargedBDT(const std::vector<std::string>& arguments)
    {
      Const::PIDDetectorSet result;
      for (std::string val : arguments) {
        boost::to_lower(val);
        if (val == "all") return Const::PIDDetectors::set();
        else if (val == "ecl") result += Const::ECL;
        else B2ERROR("Invalid detector component: " << val << " for charged BDT.");
      }
      return result;
    }

    //*************
    // Belle II
    //*************

    // a "smart" variable:
    // finds the global probability based on the PDG code of the input particle
    double particleID(const Particle* p)
    {
      int pdg = abs(p->getPDGCode());
      if (pdg == Const::electron.getPDGCode())      return electronID(p);
      else if (pdg == Const::muon.getPDGCode())     return muonID(p);
      else if (pdg == Const::pion.getPDGCode())     return pionID(p);
      else if (pdg == Const::kaon.getPDGCode())     return kaonID(p);
      else if (pdg == Const::proton.getPDGCode())   return protonID(p);
      else if (pdg == Const::deuteron.getPDGCode()) return deuteronID(p);
      else return std::numeric_limits<float>::quiet_NaN();
    }

    Manager::FunctionPtr pidLogLikelihoodValueExpert(const std::vector<std::string>& arguments)
    {
      if (arguments.size() < 2) {
        B2ERROR("Need at least two arguments to pidLogLikelihoodValueExpert");
        return nullptr;
      }
      int pdgCode;
      try {
        pdgCode = Belle2::convertString<int>(arguments[0]);
      } catch (std::invalid_argument& e) {
        B2ERROR("First argument of pidLogLikelihoodValueExpert must be a PDG code");
        return nullptr;
      }
      std::vector<std::string> detectors(arguments.begin() + 1, arguments.end());

      Const::PIDDetectorSet detectorSet = parseDetectors(detectors);
      auto hypType = Const::ChargedStable(abs(pdgCode));

      auto func = [hypType, detectorSet](const Particle * part) -> double {
        const PIDLikelihood* pid = part->getPIDLikelihood();
        if (!pid)
          return std::numeric_limits<float>::quiet_NaN();
        // No information form any subdetector in the list
        if (pid->getLogL(hypType, detectorSet) == 0)
          return std::numeric_limits<float>::quiet_NaN();

        return pid->getLogL(hypType, detectorSet);
      };
      return func;
    }



    Manager::FunctionPtr pidDeltaLogLikelihoodValueExpert(const std::vector<std::string>& arguments)
    {
      if (arguments.size() < 3) {
        B2ERROR("Need at least three arguments to pidDeltaLogLikelihoodValueExpert");
        return nullptr;
      }
      int pdgCodeHyp, pdgCodeTest;
      try {
        pdgCodeHyp = Belle2::convertString<int>(arguments[0]);
      } catch (std::invalid_argument& e) {
        B2ERROR("First argument of pidDeltaLogLikelihoodValueExpert must be a PDG code");
        return nullptr;
      }
      try {
        pdgCodeTest = Belle2::convertString<int>(arguments[1]);
      } catch (std::invalid_argument& e) {
        B2ERROR("Second argument of pidDeltaLogLikelihoodValueExpert must be a PDG code");
        return nullptr;
      }

      std::vector<std::string> detectors(arguments.begin() + 2, arguments.end());
      Const::PIDDetectorSet detectorSet = parseDetectors(detectors);
      auto hypType = Const::ChargedStable(abs(pdgCodeHyp));
      auto testType = Const::ChargedStable(abs(pdgCodeTest));

      auto func = [hypType, testType, detectorSet](const Particle * part) -> double {
        const PIDLikelihood* pid = part->getPIDLikelihood();
        if (!pid) return std::numeric_limits<float>::quiet_NaN();
        // No information form any subdetector in the list
        if (pid->getLogL(hypType, detectorSet) == 0)
          return std::numeric_limits<float>::quiet_NaN();

        return (pid->getLogL(hypType, detectorSet) - pid->getLogL(testType, detectorSet));
      };
      return func;
    }


    Manager::FunctionPtr pidPairProbabilityExpert(const std::vector<std::string>& arguments)
    {
      if (arguments.size() < 3) {
        B2ERROR("Need at least three arguments to pidPairProbabilityExpert");
        return nullptr;
      }
      int pdgCodeHyp = 0, pdgCodeTest = 0;
      try {
        pdgCodeHyp = Belle2::convertString<int>(arguments[0]);
      } catch (std::invalid_argument& e) {
        B2ERROR("First argument of pidPairProbabilityExpert must be PDG code");
        return nullptr;
      }
      try {
        pdgCodeTest = Belle2::convertString<int>(arguments[1]);
      } catch (std::invalid_argument& e) {
        B2ERROR("Second argument of pidPairProbabilityExpert must be PDG code");
        return nullptr;
      }

      std::vector<std::string> detectors(arguments.begin() + 2, arguments.end());

      Const::PIDDetectorSet detectorSet = parseDetectors(detectors);
      auto hypType = Const::ChargedStable(abs(pdgCodeHyp));
      auto testType = Const::ChargedStable(abs(pdgCodeTest));
      auto func = [hypType, testType, detectorSet](const Particle * part) -> double {
        const PIDLikelihood* pid = part->getPIDLikelihood();
        if (!pid) return std::numeric_limits<float>::quiet_NaN();
        // No information from any subdetector in the list
        if (pid->getLogL(hypType, detectorSet) == 0)
          return std::numeric_limits<float>::quiet_NaN();

        return pid->getProbability(hypType, testType, detectorSet);
      };
      return func;
    }


    Manager::FunctionPtr pidProbabilityExpert(const std::vector<std::string>& arguments)
    {
      if (arguments.size() < 2) {
        B2ERROR("Need at least two arguments for pidProbabilityExpert");
        return nullptr;
      }
      int pdgCodeHyp = 0;
      try {
        pdgCodeHyp = Belle2::convertString<int>(arguments[0]);
      } catch (std::invalid_argument& e) {
        B2ERROR("First argument of pidProbabilityExpert must be PDG code");
        return nullptr;
      }

      std::vector<std::string> detectors(arguments.begin() + 1, arguments.end());
      Const::PIDDetectorSet detectorSet = parseDetectors(detectors);
      auto hypType = Const::ChargedStable(abs(pdgCodeHyp));

      // Placeholder for the priors
      const unsigned int n = Const::ChargedStable::c_SetSize;
      double frac[n];
      for (double& i : frac) i = 1.0;  // flat priors

      auto func = [hypType, frac, detectorSet](const Particle * part) -> double {
        const PIDLikelihood* pid = part->getPIDLikelihood();
        if (!pid) return std::numeric_limits<float>::quiet_NaN();
        // No information from any subdetector in the list
        if (pid->getLogL(hypType, detectorSet) == 0)
          return std::numeric_limits<float>::quiet_NaN();

        return pid->getProbability(hypType, frac, detectorSet);
      };
      return func;
    }


    Manager::FunctionPtr pidMissingProbabilityExpert(const std::vector<std::string>& arguments)
    {
      if (arguments.size() < 1) {
        B2ERROR("Need at least one argument to pidMissingProbabilityExpert");
        return nullptr;
      }

      std::vector<std::string> detectors(arguments.begin(), arguments.end());
      Const::PIDDetectorSet detectorSet = parseDetectors(detectors);

      auto func = [detectorSet](const Particle * part) -> double {
        const PIDLikelihood* pid = part->getPIDLikelihood();
        if (!pid) return std::numeric_limits<double>::quiet_NaN();
        if (not pid->isAvailable(detectorSet))
          return 1;
        else return 0;
      };
      return func;
    }

    double electronID(const Particle* part)
    {
      return std::get<double>(Manager::Instance().getVariable("pidProbabilityExpert(11, ALL)")->function(part));
    }

    double muonID(const Particle* part)
    {
      return std::get<double>(Manager::Instance().getVariable("pidProbabilityExpert(13, ALL)")->function(part));
    }

    double pionID(const Particle* part)
    {
      return std::get<double>(Manager::Instance().getVariable("pidProbabilityExpert(211, ALL)")->function(part));
    }

    double kaonID(const Particle* part)
    {
      return std::get<double>(Manager::Instance().getVariable("pidProbabilityExpert(321, ALL)")->function(part));
    }

    double protonID(const Particle* part)
    {
      return std::get<double>(Manager::Instance().getVariable("pidProbabilityExpert(2212, ALL)")->function(part));
    }

    double deuteronID(const Particle* part)
    {
      return std::get<double>(Manager::Instance().getVariable("pidProbabilityExpert(1000010020, ALL)")->function(part));
    }

    double binaryPID(const Particle* part, const std::vector<double>& arguments)
    {
      if (arguments.size() != 2) {
        B2ERROR("The variable binaryPID needs exactly two arguments: the PDG codes of two hypotheses.");
        return std::numeric_limits<float>::quiet_NaN();;
      }
      int pdgCodeHyp = std::abs(int(std::lround(arguments[0])));
      int pdgCodeTest = std::abs(int(std::lround(arguments[1])));
      return std::get<double>(Manager::Instance().getVariable("pidPairProbabilityExpert(" + std::to_string(
                                                                pdgCodeHyp) + ", " + std::to_string(
                                                                pdgCodeTest) + ", ALL)")->function(part));
    }

    double electronID_noSVD(const Particle* part)
    {
      // Excluding SVD for electron ID. This variable is temporary. BII-8760
      return std::get<double>(Manager::Instance().getVariable("pidProbabilityExpert(11, CDC, TOP, ARICH, ECL, KLM)")->function(part));
    }

    double muonID_noSVD(const Particle* part)
    {
      // Excluding SVD for muon ID. This variable is temporary. BII-8760
      return std::get<double>(Manager::Instance().getVariable("pidProbabilityExpert(13, CDC, TOP, ARICH, ECL, KLM)")->function(part));
    }

    double pionID_noSVD(const Particle* part)
    {
      // Excluding SVD for pion ID. This variable is temporary. BII-8760
      return std::get<double>(Manager::Instance().getVariable("pidProbabilityExpert(211, CDC, TOP, ARICH, ECL, KLM)")->function(part));
    }

    double kaonID_noSVD(const Particle* part)
    {
      // Excluding SVD for kaon ID. This variable is temporary. BII-8760
      return std::get<double>(Manager::Instance().getVariable("pidProbabilityExpert(321, CDC, TOP, ARICH, ECL, KLM)")->function(part));
    }

    double protonID_noSVD(const Particle* part)
    {
      // Excluding SVD for proton ID. This variable is temporary. BII-8760
      return std::get<double>(Manager::Instance().getVariable("pidProbabilityExpert(2212, CDC, TOP, ARICH, ECL, KLM)")->function(part));
    }

    double deuteronID_noSVD(const Particle* part)
    {
      // Excluding SVD for deuteron ID. This variable is temporary. BII-8760
      return std::get<double>(Manager::Instance().getVariable("pidProbabilityExpert(1000010020, CDC, TOP, ARICH, ECL, KLM)")->function(
                                part));
    }

    double binaryPID_noSVD(const Particle* part, const std::vector<double>& arguments)
    {
      // Excluding SVD for binary ID. This variable is temporary. BII-8760
      if (arguments.size() != 2) {
        B2ERROR("The variable binaryPID_noSVD needs exactly two arguments: the PDG codes of two hypotheses.");
        return std::numeric_limits<float>::quiet_NaN();;
      }
      int pdgCodeHyp = std::abs(int(std::lround(arguments[0])));
      int pdgCodeTest = std::abs(int(std::lround(arguments[1])));
      return std::get<double>(Manager::Instance().getVariable("pidPairProbabilityExpert(" + std::to_string(
                                                                pdgCodeHyp) + ", " + std::to_string(
                                                                pdgCodeTest) + ", CDC, TOP, ARICH, ECL, KLM)")->function(part));
    }

    double electronID_noTOP(const Particle* part)
    {
      // Excluding TOP for electron ID. This variable is temporary. BII-8444
      return std::get<double>(Manager::Instance().getVariable("pidProbabilityExpert(11, SVD, CDC, ARICH, ECL, KLM)")->function(part));
    }

    double binaryElectronID_noTOP(const Particle* part, const std::vector<double>& arguments)
    {
      // Excluding TOP for electron ID. This is temporary. BII-8444
      if (arguments.size() != 1) {
        B2ERROR("The variable binaryElectronID_noTOP needs exactly one argument: the PDG code of the test hypothesis.");
        return std::numeric_limits<float>::quiet_NaN();;
      }

      int pdgCodeHyp = Const::electron.getPDGCode();
      int pdgCodeTest = std::abs(int(std::lround(arguments[0])));

      const auto var = "pidPairProbabilityExpert(" + std::to_string(pdgCodeHyp) + ", " +
                       std::to_string(pdgCodeTest) + ", SVD, CDC, ARICH, ECL, KLM)";

      return std::get<double>(Manager::Instance().getVariable(var)->function(part));
    }

    double electronID_noSVD_noTOP(const Particle* part)
    {
      // Excluding SVD and TOP for electron ID. This variable is temporary. BII-8444, BII-8760.
      return std::get<double>(Manager::Instance().getVariable("pidProbabilityExpert(11, CDC, ARICH, ECL, KLM)")->function(part));
    }

    double binaryElectronID_noSVD_noTOP(const Particle* part, const std::vector<double>& arguments)
    {
      // Excluding SVD and TOP for electron ID. This is temporary. BII-8444, BII-8760.
      if (arguments.size() != 1) {
        B2ERROR("The variable binaryElectronID_noSVD_noTOP needs exactly one argument: the PDG code of the test hypothesis.");
        return std::numeric_limits<float>::quiet_NaN();;
      }

      int pdgCodeHyp = Const::electron.getPDGCode();
      int pdgCodeTest = std::abs(int(std::lround(arguments[0])));

      const auto var = "pidPairProbabilityExpert(" + std::to_string(pdgCodeHyp) + ", " +
                       std::to_string(pdgCodeTest) + ", CDC, ARICH, ECL, KLM)";

      return std::get<double>(Manager::Instance().getVariable(var)->function(part));
    }


    double pionID_noARICHwoECL(const Particle* part)
    {
      // remove arich if no ecl cluster + identified as kaon in arich
      const ECLCluster* cluster = part->getECLCluster();
      if (!cluster) {
        const PIDLikelihood* pid = part->getPIDLikelihood();
        if (!pid) return std::numeric_limits<float>::quiet_NaN();
        if (pid->getLogL(Const::kaon, Const::ARICH) > pid->getLogL(Const::pion, Const::ARICH)) {
          return std::get<double>(Manager::Instance().getVariable("pidProbabilityExpert(211, SVD, CDC, TOP, ECL, KLM)")->function(part));
        }
      }
      return pionID(part);
    }


    double kaonID_noARICHwoECL(const Particle* part)
    {
      // remove arich if no ecl cluster + identified as kaon in arich
      const ECLCluster* cluster = part->getECLCluster();
      if (!cluster) {
        const PIDLikelihood* pid = part->getPIDLikelihood();
        if (!pid) return std::numeric_limits<float>::quiet_NaN();
        if (pid->getLogL(Const::kaon, Const::ARICH) > pid->getLogL(Const::pion, Const::ARICH)) {
          return std::get<double>(Manager::Instance().getVariable("pidProbabilityExpert(321, SVD, CDC, TOP, ECL, KLM)")->function(part));
        }
      }
      return kaonID(part)
    }


    double binaryPID_noARICHwoECL(const Particle* part, const std::vector<double>& arguments)
    {
      // Excluding ARICH for tracks without ECL cluster and identified as heavier of the two hypotheses from binary ID.
      if (arguments.size() != 2) {
        B2ERROR("The variable binaryPID_noARICHwoECL needs exactly two arguments: the PDG codes of two hypotheses.");
        return std::numeric_limits<float>::quiet_NaN();;
      }
      int pdgCodeHyp = std::abs(int(std::lround(arguments[0])));
      int pdgCodeTest = std::abs(int(std::lround(arguments[1])));
      auto hypType = Const::ChargedStable(abs(pdgCodeHyp));
      auto testType = Const::ChargedStable(abs(pdgCodeTest));

      const ECLCluster* cluster = part->getECLCluster();
      if (!cluster) {
        const PIDLikelihood* pid = part->getPIDLikelihood();
        if (!pid) return std::numeric_limits<float>::quiet_NaN();
        double lkhdiff = pid->getLogL(hypType, Const::ARICH) - pid->getLogL(testType, Const::ARICH);
        if ((lkhdiff > 0 && pdgCodeHyp > pdgCodeTest) || (lkhdiff < 0 && pdgCodeHyp < pdgCodeTest)) {
          return std::get<double>(Manager::Instance().getVariable("pidPairProbabilityExpert(" + std::to_string(
                                                                    pdgCodeHyp) + ", " + std::to_string(
                                                                    pdgCodeTest) + ", SVD, CDC, TOP, ECL, KLM)")->function(part));
        }
      }

      return binaryPID(part, arguments);

    }



    double antineutronID(const Particle* particle)
    {
      if (particle->hasExtraInfo("nbarID")) {
        return particle->getExtraInfo("nbarID");
      } else {
        if (particle->getPDGCode() == -Const::neutron.getPDGCode()) {
          B2WARNING("The extraInfo nbarID is not registered! \n"
                    "Please use function getNbarIDMVA in modularAnalysis.");
        }
        return std::numeric_limits<float>::quiet_NaN();
      }
    }

    Manager::FunctionPtr pidChargedBDTScore(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 2) {
        B2ERROR("Need exactly two arguments for pidChargedBDTScore: pdgCodeHyp, detector");
        return nullptr;
      }

      int hypPdgId;
      try {
        hypPdgId = Belle2::convertString<int>(arguments.at(0));
      } catch (std::invalid_argument& e) {
        B2ERROR("First argument of pidChargedBDTScore must be an integer (PDG code).");
        return nullptr;
      }
      Const::ChargedStable hypType = Const::ChargedStable(hypPdgId);

      std::vector<std::string> detectors(arguments.begin() + 1, arguments.end());
      Const::PIDDetectorSet detectorSet = parseDetectorsChargedBDT(detectors);

      auto func = [hypType, detectorSet](const Particle * part) -> double {
        auto name = "pidChargedBDTScore_" + std::to_string(hypType.getPDGCode());
        for (size_t iDet(0); iDet < detectorSet.size(); ++iDet)
        {
          auto det = detectorSet[iDet];
          name += "_" + std::to_string(det);
        }
        return (part->hasExtraInfo(name)) ? part->getExtraInfo(name) : std::numeric_limits<float>::quiet_NaN();
      };
      return func;
    }

    Manager::FunctionPtr pidPairChargedBDTScore(const std::vector<std::string>& arguments)
    {
      if (arguments.size() != 3) {
        B2ERROR("Need exactly three arguments for pidPairChargedBDTScore: pdgCodeHyp, pdgCodeTest, detector.");
        return nullptr;
      }

      int hypPdgId, testPdgId;
      try {
        hypPdgId = Belle2::convertString<int>(arguments.at(0));
      } catch (std::invalid_argument& e) {
        B2ERROR("First argument of pidPairChargedBDTScore must be an integer (PDG code).");
        return nullptr;
      }
      try {
        testPdgId = Belle2::convertString<int>(arguments.at(1));
      } catch (std::invalid_argument& e) {
        B2ERROR("First argument of pidPairChargedBDTScore must be an integer (PDG code).");
        return nullptr;
      }
      Const::ChargedStable hypType = Const::ChargedStable(hypPdgId);
      Const::ChargedStable testType = Const::ChargedStable(testPdgId);

      std::vector<std::string> detectors(arguments.begin() + 2, arguments.end());
      Const::PIDDetectorSet detectorSet = parseDetectorsChargedBDT(detectors);

      auto func = [hypType, testType, detectorSet](const Particle * part) -> double {
        auto name = "pidPairChargedBDTScore_" + std::to_string(hypType.getPDGCode()) + "_VS_" + std::to_string(testType.getPDGCode());
        for (size_t iDet(0); iDet < detectorSet.size(); ++iDet)
        {
          auto det = detectorSet[iDet];
          name += "_" + std::to_string(det);
        }
        return (part->hasExtraInfo(name)) ? part->getExtraInfo(name) : std::numeric_limits<float>::quiet_NaN();
      };
      return func;
    }

    double mostLikelyPDG(const Particle* part, const std::vector<double>& arguments)
    {
      if (arguments.size() != 0 and arguments.size() != Const::ChargedStable::c_SetSize) {
        B2ERROR("Need zero or exactly " << Const::ChargedStable::c_SetSize << " arguments for pidMostLikelyPDG");
        return std::numeric_limits<double>::quiet_NaN();
      }
      double prob[Const::ChargedStable::c_SetSize];
      if (arguments.size() == 0) {
        for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) prob[i] = 1. / Const::ChargedStable::c_SetSize;
      } else {
        copy(arguments.begin(), arguments.end(), prob);
      }

      auto* pid = part->getPIDLikelihood();
      if (!pid) return std::numeric_limits<double>::quiet_NaN();
      return pid->getMostLikely(prob).getPDGCode();
    }

    bool isMostLikely(const Particle* part, const std::vector<double>& arguments)
    {
      if (arguments.size() != 0 and arguments.size() != Const::ChargedStable::c_SetSize) {
        B2ERROR("Need zero or exactly " << Const::ChargedStable::c_SetSize << " arguments for pidIsMostLikely");
        return false;
      }
      return mostLikelyPDG(part, arguments) == abs(part->getPDGCode());
    }

    //*************
    // B2BII
    //*************

    double muIDBelle(const Particle* particle)
    {
      const PIDLikelihood* pid = particle->getPIDLikelihood();
      if (!pid) return 0.5; // Belle standard

      if (pid->isAvailable(Const::KLM))
        return exp(pid->getLogL(Const::muon, Const::KLM));
      else
        return 0; // Belle standard
    }

    double muIDBelleQuality(const Particle* particle)
    {
      const PIDLikelihood* pid = particle->getPIDLikelihood();
      if (!pid) return 0;// Belle standard

      return pid->isAvailable(Const::KLM);
    }

    double atcPIDBelle(const Particle* particle,  const std::vector<double>& sigAndBkgHyp)
    {
      int sigHyp = int(std::lround(sigAndBkgHyp[0]));
      int bkgHyp = int(std::lround(sigAndBkgHyp[1]));

      const PIDLikelihood* pid = particle->getPIDLikelihood();
      if (!pid) return 0.5; // Belle standard

      // ACC = ARICH
      Const::PIDDetectorSet set = Const::ARICH;
      double acc_sig = exp(pid->getLogL(hypothesisConversion(sigHyp), set));
      double acc_bkg = exp(pid->getLogL(hypothesisConversion(bkgHyp), set));
      double acc = 0.5; // Belle standard
      if (acc_sig + acc_bkg  > 0.0)
        acc = acc_sig / (acc_sig + acc_bkg);

      // TOF = TOP
      set = Const::TOP;
      double tof_sig = exp(pid->getLogL(hypothesisConversion(sigHyp), set));
      double tof_bkg = exp(pid->getLogL(hypothesisConversion(bkgHyp), set));
      double tof = 0.5; // Belle standard
      double tof_all = tof_sig + tof_bkg;
      if (tof_all != 0) {
        tof = tof_sig / tof_all;
        if (tof < 0.001) tof = 0.001;
        if (tof > 0.999) tof = 0.999;
      }

      // dE/dx = CDC
      set = Const::CDC;
      double cdc_sig = exp(pid->getLogL(hypothesisConversion(sigHyp), set));
      double cdc_bkg = exp(pid->getLogL(hypothesisConversion(bkgHyp), set));
      double cdc = 0.5; // Belle standard
      double cdc_all = cdc_sig + cdc_bkg;
      if (cdc_all != 0) {
        cdc = cdc_sig / cdc_all;
        if (cdc < 0.001) cdc = 0.001;
        if (cdc > 0.999) cdc = 0.999;
      }

      // Combined
      double pid_sig = acc * tof * cdc;
      double pid_bkg = (1. - acc) * (1. - tof) * (1. - cdc);

      return pid_sig / (pid_sig + pid_bkg);
    }


    double eIDBelle(const Particle* part)
    {
      const PIDLikelihood* pid = part->getPIDLikelihood();
      if (!pid) return 0.5; // Belle standard

      Const::PIDDetectorSet set = Const::ECL;
      return pid->getProbability(Const::electron, Const::pion, set);
    }


    // PID variables to be used for analysis
    VARIABLE_GROUP("PID");
    REGISTER_VARIABLE("particleID", particleID,
                      "the particle identification probability under the particle's own hypothesis, using info from all available detectors");
    REGISTER_VARIABLE("electronID", electronID,
                      "electron identification probability defined as :math:`\\mathcal{L}_e/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors");
    REGISTER_VARIABLE("muonID", muonID,
                      "muon identification probability defined as :math:`\\mathcal{L}_\\mu/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors");
    REGISTER_VARIABLE("pionID", pionID,
                      "pion identification probability defined as :math:`\\mathcal{L}_\\pi/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors");
    REGISTER_VARIABLE("kaonID", kaonID,
                      "kaon identification probability defined as :math:`\\mathcal{L}_K/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors");
    REGISTER_VARIABLE("protonID", protonID,
                      "proton identification probability defined as :math:`\\mathcal{L}_p/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors");
    REGISTER_VARIABLE("deuteronID", deuteronID,
                      "deuteron identification probability defined as :math:`\\mathcal{L}_d/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors");
    REGISTER_METAVARIABLE("binaryPID(pdgCode1, pdgCode2)", binaryPID,
                          "Returns the binary probability for the first provided mass hypothesis with respect to the second mass hypothesis using all detector components",
                          Manager::VariableDataType::c_double);
    REGISTER_METAVARIABLE("pidChargedBDTScore(pdgCodeHyp, detector)", pidChargedBDTScore,
                          "Returns the charged Pid BDT score for a certain mass hypothesis with respect to all other charged stable particle hypotheses. The second argument specifies which BDT training to use: based on 'ALL' PID detectors (NB: 'SVD' is currently excluded), or 'ECL' only. The choice depends on the ChargedPidMVAMulticlassModule's configuration.",
                          Manager::VariableDataType::c_double);
    REGISTER_METAVARIABLE("pidPairChargedBDTScore(pdgCodeHyp, pdgCodeTest, detector)", pidPairChargedBDTScore,
                          "Returns the charged Pid BDT score for a certain mass hypothesis with respect to an alternative hypothesis. The second argument specifies which BDT training to use: based on 'ALL' PID detectors (NB: 'SVD' is currently excluded), or 'ECL' only. The choice depends on the ChargedPidMVAModule's configuration.",
                          Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("nbarID", antineutronID, R"DOC(
Returns MVA classifier for antineutron PID.

    - 1  signal(antineutron) like
    - 0  background like
    - -1 invalid using this PID due to some ECL variables used unavailable

This PID is only for antineutron. Neutron is also considered as background.
The variables used are `clusterPulseShapeDiscriminationMVA`, `clusterE`, `clusterLAT`, `clusterE1E9`, `clusterE9E21`,
`clusterAbsZernikeMoment40`, `clusterAbsZernikeMoment51`, `clusterZernikeMVA`.)DOC");

    // Special temporary variables defined for users' convenience.
    REGISTER_VARIABLE("electronID_noSVD", electronID_noSVD,
                      "**(SPECIAL (TEMP) variable)** electron identification probability defined as :math:`\\mathcal{L}_e/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors *excluding the SVD*");
    REGISTER_VARIABLE("muonID_noSVD", muonID_noSVD,
                      "**(SPECIAL (TEMP) variable)** muon identification probability defined as :math:`\\mathcal{L}_\\mu/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors *excluding the SVD*");
    REGISTER_VARIABLE("pionID_noSVD", pionID_noSVD,
                      "**(SPECIAL (TEMP) variable)** pion identification probability defined as :math:`\\mathcal{L}_\\pi/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors *excluding the SVD*");
    REGISTER_VARIABLE("kaonID_noSVD", kaonID_noSVD,
                      "**(SPECIAL (TEMP) variable)** kaon identification probability defined as :math:`\\mathcal{L}_K/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors *excluding the SVD*");
    REGISTER_VARIABLE("protonID_noSVD", protonID_noSVD,
                      "**(SPECIAL (TEMP) variable)** proton identification probability defined as :math:`\\mathcal{L}_p/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors *excluding the SVD*");
    REGISTER_VARIABLE("deuteronID_noSVD", deuteronID_noSVD,
                      "**(SPECIAL (TEMP) variable)** deuteron identification probability defined as :math:`\\mathcal{L}_d/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors *excluding the SVD*");
    REGISTER_METAVARIABLE("binaryPID_noSVD(pdgCode1, pdgCode2)", binaryPID_noSVD,
                          "Returns the binary probability for the first provided mass hypothesis with respect to the second mass hypothesis using all detector components, *excluding the SVD*.",
                          Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("electronID_noTOP", electronID_noTOP,
                      "**(SPECIAL (TEMP) variable)** electron identification probability defined as :math:`\\mathcal{L}_e/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors *excluding the TOP*. *NB:* this variable must be used in place of `electronID` when analysing data (MC) processed (simulated) in *release 6*");
    REGISTER_METAVARIABLE("binaryElectronID_noTOP(pdgCodeTest)", binaryElectronID_noTOP,
                          "**(SPECIAL (TEMP) variable)** Returns the binary probability for the electron mass hypothesis with respect to another mass hypothesis using all detector components, *excluding the TOP*. *NB:* this variable must be used in place of `binaryPID` (``pdgCode1=11``) when analysing data (MC) processed (simulated) in **release 6**",
                          Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("electronID_noSVD_noTOP", electronID_noSVD_noTOP,
                      "**(SPECIAL (TEMP) variable)** electron identification probability defined as :math:`\\mathcal{L}_e/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors *excluding the SVD and the TOP*. *NB:* this variable must be used in place of `electronID` when analysing data (MC) processed (simulated) in *release 5*");
    REGISTER_METAVARIABLE("binaryElectronID_noSVD_noTOP(pdgCodeTest)", binaryElectronID_noSVD_noTOP,
                          "**(SPECIAL (TEMP) variable)** Returns the binary probability for the electron mass hypothesis with respect to another mass hypothesis using all detector components, *excluding the SVD and the TOP*. *NB:* this variable must be used in place of `binaryPID` (``pdgCode1=11``) when analysing data (MC) processed (simulated) in **release 5**",
                          Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("pionID_noARICHwoECL", pionID_noARICHwoECL,
                      "**(SPECIAL (TEMP) variable)** pion identification probability defined as :math:`\\mathcal{L}_\\pi/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors but ARICH info excluded for tracks without associated ECL cluster");
    REGISTER_VARIABLE("kaonID_noARICHwoECL", kaonID_noARICHwoECL,
                      "**(SPECIAL (TEMP) variable)** kaon identification probability defined as :math:`\\mathcal{L}_K/(\\mathcal{L}_e+\\mathcal{L}_\\mu+\\mathcal{L}_\\pi+\\mathcal{L}_K+\\mathcal{L}_p+\\mathcal{L}_d)`, using info from all available detectors but ARICH info excluded for tracks without associated ECL cluster");
    REGISTER_METAVARIABLE("binaryPID_noARICHwoECL(pdgCode1, pdgCode2)", binaryPID_noARICHwoECL,
                          "Returns the binary probability for the first provided mass hypothesis with respect to the second mass hypothesis using all detector components, but ARICH info excluded for tracks without associated ECL cluster",
                          Manager::VariableDataType::c_double);

    // Metafunctions for experts to access the basic PID quantities
    VARIABLE_GROUP("PID_expert");
    REGISTER_METAVARIABLE("pidLogLikelihoodValueExpert(pdgCode, detectorList)", pidLogLikelihoodValueExpert,
                          "returns the log likelihood value of for a specific mass hypothesis and  set of detectors.", Manager::VariableDataType::c_double);
    REGISTER_METAVARIABLE("pidDeltaLogLikelihoodValueExpert(pdgCode1, pdgCode2, detectorList)", pidDeltaLogLikelihoodValueExpert,
                          "returns LogL(hyp1) - LogL(hyp2) (aka DLL) for two mass hypotheses and a set of detectors.", Manager::VariableDataType::c_double);
    REGISTER_METAVARIABLE("pidPairProbabilityExpert(pdgCodeHyp, pdgCodeTest, detectorList)", pidPairProbabilityExpert,
                          "Pair (or binary) probability for the pdgCodeHyp mass hypothesis respect to the pdgCodeTest one, using an arbitrary set of detectors. :math:`\\mathcal{L}_{hyp}/(\\mathcal{L}_{test}+\\mathcal{L}_{hyp}`",
                          Manager::VariableDataType::c_double);
    REGISTER_METAVARIABLE("pidProbabilityExpert(pdgCodeHyp, detectorList)", pidProbabilityExpert,
                          "probability for the pdgCodeHyp mass hypothesis respect to all the other ones, using an arbitrary set of detectors :math:`\\mathcal{L}_{hyp}/(\\Sigma_{\\text{all~hyp}}\\mathcal{L}_{i}`. ",
                          Manager::VariableDataType::c_double);
    REGISTER_METAVARIABLE("pidMissingProbabilityExpert(detectorList)", pidMissingProbabilityExpert,
                          "returns 1 if the PID probabiliy is missing for the provided detector list, otherwise 0. ", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("pidMostLikelyPDG(ePrior=1/6, muPrior=1/6, piPrior=1/6, KPrior=1/6, pPrior=1/6, dPrior=1/6)", mostLikelyPDG,
                      R"DOC(
Returns PDG code of the largest PID likelihood, or NaN if PID information is not available.
This function accepts either no arguments, or 6 floats as priors for the charged particle hypotheses
following the order shown in the metavariable's declaration. Flat priors are assumed as default.)DOC");
    REGISTER_VARIABLE("pidIsMostLikely(ePrior=1/6, muPrior=1/6, piPrior=1/6, KPrior=1/6, pPrior=1/6, dPrior=1/6)", isMostLikely, R"DOC(
Returns True if the largest PID likelihood of a given particle corresponds to its particle hypothesis.
This function accepts either no arguments, or 6 floats as priors for the charged particle hypotheses
following the order shown in the metavariable's declaration. Flat priors are assumed as default.)DOC");

    // B2BII PID
    VARIABLE_GROUP("Belle PID variables");
    REGISTER_METAVARIABLE("atcPIDBelle(i,j)", atcPIDBelle, R"DOC(
[Legacy] Returns Belle's PID atc variable: ``atc_pid(3,1,5,i,j).prob()``.
Parameters i,j are signal and background hypothesis: (0 = electron, 1 = muon, 2 = pion, 3 = kaon, 4 = proton)
Returns 0.5 in case there is no likelihood found and a factor of 0.5 will appear in the product if any of the subdetectors don't report a likelihood (Belle behaviour).

.. warning:: The behaviour is different from Belle II PID variables which typically return NaN in case of error.
    )DOC", Manager::VariableDataType::c_double);
    REGISTER_VARIABLE("muIDBelle", muIDBelle, R"DOC(
[Legacy] Returns Belle's PID ``Muon_likelihood()`` variable.
Returns 0.5 in case there is no likelihood found and returns zero if the muon likelihood is not usable (Belle behaviour).

.. warning:: The behaviour is different from Belle II PID variables which typically return NaN in case of error.
    )DOC");
    REGISTER_VARIABLE("muIDBelleQuality", muIDBelleQuality, R"DOC(
[Legacy] Returns true if Belle's PID ``Muon_likelihood()`` is usable (reliable).
Returns zero/false if not usable or if there is no PID found.
    )DOC");
    REGISTER_VARIABLE("eIDBelle", eIDBelle, R"DOC(
[Legacy] Returns Belle's electron ID ``eid(3,-1,5).prob()`` variable.
Returns 0.5 in case there is no likelihood found (Belle behaviour).

.. warning:: The behaviour is different from Belle II PID variables which typically return NaN in case of error.
    )DOC");
  }
}
