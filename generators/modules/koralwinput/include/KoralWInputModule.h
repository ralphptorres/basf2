/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KoralW header. */
#include <generators/koralw/KoralW.h>

/* Basf2 headers. */
#include <generators/modules/GeneratorBaseModule.h>
#include <generators/utilities/InitialParticleGeneration.h>
#include <mdst/dataobjects/MCParticleGraph.h>

/* C++ headers. */
#include <string>

namespace Belle2 {

  /**
   * The KoralW Generator module.
   * Generates four fermion final state events using the KoralW FORTRAN generator.
   */
  class KoralWInputModule : public GeneratorBaseModule {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    KoralWInputModule();

    /** Destructor. */
    virtual ~KoralWInputModule();

    /** Initializes the module. */
    void initialize() override;

    /** Method is called for each event. */
    void generatorEvent() override;

    /** Method is called at the end of the event processing. */
    void terminate() override;

    /** Convert m_eventType from string to int */
    double getEventType() const override
    {
      if (m_eventType == "e+e-e+e-") return 11111111;
      if (m_eventType == "e+e-mu+mu-") return 11111313;
      if (m_eventType == "e+e-tau+tau-") return 11111515;

      if (m_eventType == "mu+mu-mu+mu-") return 13131313;
      if (m_eventType == "mu+mu-tau+tau-") return 13131515;
      if (m_eventType == "tau+tau-tau+tau-") return 15151515;

      return Const::doubleNaN;
    };

  private:

    std::string m_dataPath; /**< The path to the KoralW input data files. */
    std::string m_userDataFile; /**< The filename of the user KoralW input data file. */
    bool m_initialized{false}; /**< True if generator has been initialized. */
    bool m_firstEvent{true}; /**< Flag for keeping track of the first call of the event() method. */
    KoralW m_generator; /**< The KoralW generator. */
    MCParticleGraph m_mcGraph; /**< The MCParticle graph object. */
    DBObjPtr<BeamParameters> m_beamParams; /**< BeamParameter database object. */
    InitialParticleGeneration m_initial; /**< InitialParticleGeneration utility. */

  };

} // end namespace Belle2
