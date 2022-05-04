/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <analysis/variables/ParticleDaughterVariables.h>

// include VariableManager
#include <analysis/VariableManager/Manager.h>

#include <analysis/variables/MCTruthVariables.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/MCParticle.h>

// framework aux
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

#include <iostream>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace Variable {

    double hasCharmedDaughter(const Particle* particle, const std::vector<double>& transition)
    {
      double Status = 0.0;

      // Check if correct arguments
      if (abs(transition[0]) != 1) {
        B2ERROR("The parameter variable hasCharmedDaughter() only accepts 1 or -1 as an argument.");
        return std::numeric_limits<float>::quiet_NaN();
      }

      // Check if particle exists
      if (!particle) {
        B2ERROR("This particle does not exist!");
        return std::numeric_limits<float>::quiet_NaN();
      }

      // Check if MC particle exists
      const MCParticle* mcp = particle->getMCParticle();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();

      // MCParticle should be related to a B meson
      if (abs(mcp->getPDG()) != 511 and abs(mcp->getPDG()) != 521)
        return std::numeric_limits<float>::quiet_NaN();

      // Check if the particle has daughters
      int nDaughters = int(mcp->getNDaughters());
      if (nDaughters < 1) {
        B2ERROR("This particle does not have any daughters!");
        return std::numeric_limits<float>::quiet_NaN();
      }

      // Get the PDG sign and load daughters
      int motherPDGSign = (particle->getPDGCode()) / (abs(particle->getPDGCode()));
      std::vector<MCParticle*> mcDaughters = mcp->getDaughters();

      for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) {
        int daughterPDG = mcDaughters[iDaughter]->getPDG();
        int daughterPDGSign = daughterPDG / (abs(daughterPDG));

        if (transition[0] == 1) {
          if (((abs(daughterPDG) / 100) % 10 == 4 || (abs(daughterPDG) / 1000) % 10 == 4)
              && motherPDGSign == daughterPDGSign) // charmed meson or baryon and b->anti-c transition
            Status = 1.0;
        } else if (transition[0] == -1) {
          if (((abs(daughterPDG) / 100) % 10 == 4 || (abs(daughterPDG) / 1000) % 10 == 4)
              && motherPDGSign == -daughterPDGSign) // charmed meson or baryon and b->c transition
            Status = 1.0;
        }
      }

      return Status;
    }

    double hasCharmoniumDaughter(const Particle* particle)
    {
      double Status = 0.0;

      // Check if particle exists
      if (!particle) {
        B2ERROR("This particle does not exist!");
        return std::numeric_limits<float>::quiet_NaN();
      }

      // Check if MC particle exists
      const MCParticle* mcp = particle->getMCParticle();
      if (!mcp)
        return std::numeric_limits<float>::quiet_NaN();

      // MCParticle should be related to a B meson
      if (abs(mcp->getPDG()) != 511 and abs(mcp->getPDG()) != 521)
        return std::numeric_limits<float>::quiet_NaN();

      // Check if the particle has daughters
      int nDaughters = int(mcp->getNDaughters());
      if (nDaughters < 1) {
        B2ERROR("This particle does not have any daughters!");
        return std::numeric_limits<float>::quiet_NaN();
      }

      // Load daughters
      std::vector<MCParticle*> mcDaughters = mcp->getDaughters();

      for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) {
        int daughterPDG = mcDaughters[iDaughter]->getPDG();
        if ((abs(daughterPDG) / 10) % 10 == 4 && (abs(daughterPDG) / 100) % 10 == 4) // charmonium state: b->c anti-c q transition
          Status = 1.0;
      }

      return Status;
    }

    double hasRealPhotonDaughter(const Particle* particle)
    {
      double Status = 0.0;

      // Check if particle exists
      if (!particle) {
        B2ERROR("This particle does not exist!");
        return std::numeric_limits<float>::quiet_NaN();
      }

      // Check if the particle has daughters
      int nDaughters = int(particle->getNDaughters());
      if (nDaughters < 1) {
        B2ERROR("This particle does not have any daughters!");
        return std::numeric_limits<float>::quiet_NaN();
      }

      // Load daughters
      const std::vector<Particle*> daughters = particle->getDaughters();

      for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) {
        double photosFlag = particleMCPhotosParticle(daughters[iDaughter]);
        int PDGcode = daughters[iDaughter]->getPDGCode();

        // Is it a real photon?
        if (PDGcode == Const::photon.getPDGCode() && photosFlag > -0.5 && photosFlag < 0.5) {  // must not be from PHOTOS
          Status = 1.0;
        }
      }

      return Status;
    }

    VARIABLE_GROUP("DirectDaughterInfo");
    REGISTER_VARIABLE("hasCharmedDaughter(i)", hasCharmedDaughter,
                      "The variable determines whether the particle's MC partner, which has to be a B-meson, has at least one daughter with a charm or anti-charm quark.\n"
                      "Returns 1 if the process is a b -> anti-c / anti-b -> c transition and -1 if the process is a b -> c / anti-b -> anti-c transition.\n"
                      "Returns 0 if the particle's MC partner does not have any charm or anti-charm quark.")
    REGISTER_VARIABLE("hasCharmoniumDaughter", hasCharmoniumDaughter,
                      "Returns 1 if on MC truth level a b -> c anti-c q transition was generated, otherwise returns 0. The particle's MC partner must be a B-meson.\n");
    REGISTER_VARIABLE("hasRealPhotonDaughter", hasRealPhotonDaughter,
                      "Returns 1 if on MC truth level there is at least one real photon daughter, a photon that was not created by photos, otherwise returns 0.");
  }
}
