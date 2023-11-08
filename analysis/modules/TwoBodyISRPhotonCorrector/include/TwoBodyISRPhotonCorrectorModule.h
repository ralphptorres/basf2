/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//#pragma once
#include <framework/core/Module.h>

#include <framework/datastore/StoreObjPtr.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <string>

namespace Belle2 {

  /** This module corrects the energy and momentum of high energy
  ISR photons in single ISR events based on the beam energy,
  photon direction, and mass of the recoil particle.  */

  class TwoBodyISRPhotonCorrectorModule : public Module {
  private:
    /** output particleList */
    StoreObjPtr<ParticleList> m_outputGammaList;
    /** name of input particle list. */
    std::string m_inputGammaListName;
    /** name of output particle list. */
    std::string m_outputGammaListName;
    /** PDG code of particle constraining the gamma energy */
    Int_t m_massiveParticlePDGCode;
    /** PDG code of output particle list. Should be 22 (photon)*/
    Int_t m_outputGammaPDGCode;

  public:
    /** Constructor. */
    TwoBodyISRPhotonCorrectorModule();

    /** Initialises the module.
     */
    virtual void initialize() override;
    /** Method called for each event. */
    virtual void event() override;
  };
} // end namespace Belle2


