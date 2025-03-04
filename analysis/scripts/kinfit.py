#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from basf2 import register_module

import pdg


def fitKinematic4C(
    list_name,
    fitterEngine='NewFitterGSL',
    constraint='HardBeam',
    daughtersUpdate=True,
    addUnmeasuredPhoton=False,
    variablePrefix="",
    decayStringForDirectionOnlyParticles="",
    decayStringForAlternateMassParticles="",
    alternateMassHypos=[],
    decayStringForNeutronVsAntiNeutron="",
    path=None,
):
    """
    Perform a 4C momentum constraint kinematic fit for particles in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param fitterEngine which fitter engine to use? 'NewFitterGSL' or 'OPALFitterGSL'
    @param constraint       HardBeam or RecoilMass
    @param daughtersUpdate make copy of the daughters and update them after the vertex fit
    @param addUnmeasuredPhoton add one unmeasured photon (uses up three constraints)
    @param variablePrefix prepended to fit variables stored in extra info. Required if
        ParticleKinematicFitter is run multiple times
    @param decayStringForDirectionOnlyParticles DecayString specifying the particles
        to use only direction information in the fit
    @param decayStringForAlternateMassParticles DecayString specifying the particles
        where an alternate mass hypothesis is used
    @param alternateMassHypos list of pdg values (or particle names) for particles where
        different mass hypothesis is used in the fit
    @param decayStringForNeutronVsAntiNeutron DecayString specifying the charged particle
        used to tag whether n or nbar. When tag particle has negative charge, PDG sign of n/nbar
        is flipped from default given in alternateMassHypos
    @param path         modules are added to this path
    """

    orca = register_module('ParticleKinematicFitter')
    orca.set_name('ParticleKinematicFitter_' + list_name)
    orca.param('debugFitter', False)
    orca.param('orcaTracer', 'None')
    orca.param('orcaFitterEngine', fitterEngine)
    orca.param('orcaConstraint', constraint)  # beam parameters automatically taken from database
    orca.param('listName', list_name)
    orca.param('updateDaughters', daughtersUpdate)
    orca.param('addUnmeasuredPhoton', addUnmeasuredPhoton)
    orca.param('variablePrefix', variablePrefix)
    orca.param('decayStringForDirectionOnlyParticles', decayStringForDirectionOnlyParticles)
    orca.param('decayStringForAlternateMassParticles', decayStringForAlternateMassParticles)
    orca.param('decayStringForNeutronVsAntiNeutron', decayStringForNeutronVsAntiNeutron)
    orca.param('alternateMassHypos', pdg.from_names(alternateMassHypos))
    path.add_module(orca)


def UnmeasuredfitKinematic1C(
    list_name,
    fitterEngine='NewFitterGSL',
    constraint='HardBeam',
    daughtersUpdate=True,
    variablePrefix="",
    decayStringForDirectionOnlyParticles="",
    decayStringForAlternateMassParticles="",
    alternateMassHypos=[],
    decayStringForNeutronVsAntiNeutron="",
    path=None,
):
    """
    Perform 1C momentum constraint kinematic fit with one unmeasured photon for particles in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param fitterEngine which fitter engine to use? 'NewFitterGSL' or 'OPALFitterGSL'
    @param constraint       HardBeam or RecoilMass
    @param daughtersUpdate make copy of the daughters and update them after the vertex fit
    @param variablePrefix prepended to fit variables stored in extra info. Required if ParticleKinematicFitter is run multiple times
    @param decayStringForDirectionOnlyParticles DecayString specifying the particles
        to use only direction information in the fit
    @param decayStringForAlternateMassParticles DecayString specifying the particles
        where an alternate mass hypothesis is used
    @param alternateMassHypos list of pdg values (or particle names) for particles where
        different mass hypothesis is used in the fit
    @param decayStringForNeutronVsAntiNeutron DecayString specifying the charged particle
        used to tag whether n or nbar. When tag particle has negative charge, PDG sign of n/nbar
        is flipped from default given in alternateMassHypos
    @param path         modules are added to this path
    """

    orca = register_module('ParticleKinematicFitter')
    orca.set_name('ParticleKinematicFitter_' + list_name)
    orca.param('debugFitter', False)
    orca.param('orcaTracer', 'None')
    orca.param('orcaFitterEngine', fitterEngine)
    orca.param('orcaConstraint', constraint)  # beam parameters automatically taken from database
    orca.param('listName', list_name)
    orca.param('updateDaughters', daughtersUpdate)
    orca.param('addUnmeasuredPhoton', True)
    orca.param('variablePrefix', variablePrefix)
    orca.param('decayStringForDirectionOnlyParticles', decayStringForDirectionOnlyParticles)
    orca.param('decayStringForAlternateMassParticles', decayStringForAlternateMassParticles)
    orca.param('decayStringForNeutronVsAntiNeutron', decayStringForNeutronVsAntiNeutron)
    orca.param('alternateMassHypos', pdg.from_names(alternateMassHypos))
    path.add_module(orca)


def fitKinematic3C(
        list_name,
        fitterEngine='NewFitterGSL',
        constraint='HardBeam',
        daughtersUpdate=True,
        addUnmeasuredPhoton=False,
        add3CPhoton=True,
        variablePrefix="",
        decayStringForDirectionOnlyParticles="",
        decayStringForAlternateMassParticles="",
        alternateMassHypos=[],
        decayStringForNeutronVsAntiNeutron="",
        path=None,
):
    """
    Perform 3C momentum constraint kinematic fit with one photon with unmeasured energy for particles
    in the given ParticleList, the first daughter should be the energy unmeasured Photon.

    @param list_name    name of the input ParticleList
    @param fitterEngine which fitter engine to use? 'NewFitterGSL' or 'OPALFitterGSL'
    @param constraint       HardBeam or RecoilMass
    @param daughtersUpdate make copy of the daughters and update them after the vertex fit
    @param addUnmeasuredPhoton add one unmeasured photon (uses up three constraints)
    @param add3CPhoton add one photon with unmeasured energy (uses up a constraint)
    @param variablePrefix prepended to fit variables stored in extra info. Required if ParticleKinematicFitter is run multiple times
    @param decayStringForDirectionOnlyParticles DecayString specifying the particles
        to use only direction information in the fit
    @param decayStringForAlternateMassParticles DecayString specifying the particles
        where an alternate mass hypothesis is used
    @param alternateMassHypos list of pdg values (or particle names) for particles where
        different mass hypothesis is used in the fit
    @param decayStringForNeutronVsAntiNeutron DecayString specifying the charged particle
        used to tag whether n or nbar. When tag particle has negative charge, PDG sign of n/nbar
        is flipped from default given in alternateMassHypos
    @param path         modules are added to this path
    """

    orca = register_module('ParticleKinematicFitter')
    orca.set_name('ParticleKinematicFitter_' + list_name)
    orca.param('debugFitter', False)
    orca.param('orcaTracer', 'None')
    orca.param('orcaFitterEngine', fitterEngine)
    orca.param('orcaConstraint', constraint)  # beam parameters automatically taken from database
    orca.param('listName', list_name)
    orca.param('updateDaughters', daughtersUpdate)
    orca.param('addUnmeasuredPhoton', addUnmeasuredPhoton)
    orca.param('add3CPhoton', add3CPhoton)
    orca.param('variablePrefix', variablePrefix)
    orca.param('decayStringForDirectionOnlyParticles', decayStringForDirectionOnlyParticles)
    orca.param('decayStringForAlternateMassParticles', decayStringForAlternateMassParticles)
    orca.param('decayStringForNeutronVsAntiNeutron', decayStringForNeutronVsAntiNeutron)
    orca.param('alternateMassHypos', pdg.from_names(alternateMassHypos))
    path.add_module(orca)


def fitKinematic2C(
        list_name,
        fitterEngine='NewFitterGSL',
        constraint='HardBeam',
        daughtersUpdate=True,
        addUnmeasuredPhotonAlongBeam="",
        variablePrefix="",
        decayStringForDirectionOnlyParticles="",
        decayStringForAlternateMassParticles="",
        alternateMassHypos=[],
        decayStringForNeutronVsAntiNeutron="",
        path=None,
):
    """
    Perform 2C momentum constraint kinematic fit. The photon with unmeasured energy and theta
    has to be the first particle in the decay string. If 'addUnmeasuredPhotonAlongBeam' is set to
    'HER' or 'LER', both phi and theta (treated as measured) of this photon are then used. Concurrently,
    an additional unmeasured photon along HER/LER will be taken into account in the fit, which means
    the momentum is only constrained in the plane perpendicular to one of the beams.

    @param list_name    name of the input ParticleList
    @param fitterEngine which fitter engine to use? 'NewFitterGSL' or 'OPALFitterGSL'
    @param constraint       HardBeam or RecoilMass
    @param daughtersUpdate make copy of the daughters and update them after the vertex fit
    @param addUnmeasuredPhotonAlongBeam add an unmeasured photon along beam if 'HER' or 'LER' is set
    @param variablePrefix prepended to fit variables stored in extra info. Required if ParticleKinematicFitter is run multiple times
    @param decayStringForDirectionOnlyParticles DecayString specifying the particles
        to use only direction information in the fit
    @param decayStringForAlternateMassParticles DecayString specifying the particles
        where an alternate mass hypothesis is used
    @param alternateMassHypos list of pdg values (or particle names) for particles where
        different mass hypothesis is used in the fit
    @param decayStringForNeutronVsAntiNeutron DecayString specifying the charged particle
        used to tag whether n or nbar. When tag particle has negative charge, PDG sign of n/nbar
        is flipped from default given in alternateMassHypos
    @param path         modules are added to this path
    """

    # Parameter check
    assert addUnmeasuredPhotonAlongBeam in ["", "LER", "HER"]

    orca = register_module('ParticleKinematicFitter')
    orca.set_name('ParticleKinematicFitter_' + list_name)
    orca.param('debugFitter', False)
    orca.param('orcaTracer', 'None')
    orca.param('orcaFitterEngine', fitterEngine)
    orca.param('orcaConstraint', constraint)  # beam parameters automatically taken from database
    orca.param('listName', list_name)
    orca.param('updateDaughters', daughtersUpdate)
    orca.param('add3CPhoton', True)
    if addUnmeasuredPhotonAlongBeam == "":
        orca.param('liftPhotonTheta', True)
    else:
        orca.param('addUnmeasuredPhoton', True)
        if addUnmeasuredPhotonAlongBeam == "HER":
            orca.param('fixUnmeasuredToHER', True)
        else:  # should be LER
            orca.param('fixUnmeasuredToLER', True)
    orca.param('variablePrefix', variablePrefix)
    orca.param('decayStringForDirectionOnlyParticles', decayStringForDirectionOnlyParticles)
    orca.param('decayStringForAlternateMassParticles', decayStringForAlternateMassParticles)
    orca.param('decayStringForNeutronVsAntiNeutron', decayStringForNeutronVsAntiNeutron)
    orca.param('alternateMassHypos', pdg.from_names(alternateMassHypos))
    path.add_module(orca)


def MassfitKinematic1CRecoil(
    list_name,
    recoilMass,
    fitterEngine='NewFitterGSL',
    constraint='RecoilMass',
    daughtersUpdate=True,
    variablePrefix="",
    decayStringForDirectionOnlyParticles="",
    decayStringForAlternateMassParticles="",
    alternateMassHypos=[],
    decayStringForNeutronVsAntiNeutron="",
    path=None,
):
    """
    Perform recoil mass kinematic fit for particles in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param fitterEngine which fitter engine to use? 'NewFitterGSL' or 'OPALFitterGSL'
    @param constraint       HardBeam or RecoilMass
    @param recoilMass       RecoilMass (GeV)
    @param daughtersUpdate make copy of the daughters and update them after the vertex fit
    @param variablePrefix prepended to fit variables stored in extra info. Required if ParticleKinematicFitter is run multiple times
    @param decayStringForDirectionOnlyParticles DecayString specifying the particles
        to use only direction information in the fit
    @param decayStringForAlternateMassParticles DecayString specifying the particles
        where an alternate mass hypothesis is used
    @param alternateMassHypos list of pdg values (or particle names) for particles where
        different mass hypothesis is used in the fit
    @param decayStringForNeutronVsAntiNeutron DecayString specifying the charged particle
        used to tag whether n or nbar. When tag particle has negative charge, PDG sign of n/nbar
        is flipped from default given in alternateMassHypos
    @param path         modules are added to this path
    """

    orca = register_module('ParticleKinematicFitter')
    orca.set_name('ParticleKinematicFitter_' + list_name)
    orca.param('debugFitter', False)
    orca.param('orcaTracer', 'None')
    orca.param('orcaFitterEngine', fitterEngine)
    orca.param('orcaConstraint', constraint)
    orca.param('recoilMass', recoilMass)
    orca.param('listName', list_name)
    orca.param('updateDaughters', daughtersUpdate)
    orca.param('addUnmeasuredPhoton', False)
    orca.param('variablePrefix', variablePrefix)
    orca.param('decayStringForDirectionOnlyParticles', decayStringForDirectionOnlyParticles)
    orca.param('decayStringForAlternateMassParticles', decayStringForAlternateMassParticles)
    orca.param('decayStringForNeutronVsAntiNeutron', decayStringForNeutronVsAntiNeutron)
    orca.param('alternateMassHypos', pdg.from_names(alternateMassHypos))
    path.add_module(orca)


def MassfitKinematic1C(
    list_name,
    invMass,
    fitterEngine='NewFitterGSL',
    constraint='Mass',
    daughtersUpdate=True,
    variablePrefix="",
    decayStringForDirectionOnlyParticles="",
    decayStringForAlternateMassParticles="",
    alternateMassHypos=[],
    decayStringForNeutronVsAntiNeutron="",
    path=None,
):
    """
    Perform recoil mass kinematic fit for particles in the given ParticleList.

    @param list_name    name of the input ParticleList
    @param fitterEngine which fitter engine to use? 'NewFitterGSL' or 'OPALFitterGSL'
    @param constraint       HardBeam or RecoilMass or Mass
    @param invMass       Invariant Mass (GeV)
    @param daughtersUpdate make copy of the daughters and update them after the vertex fit
    @param variablePrefix prepended to fit variables stored in extra info. Required if ParticleKinematicFitter is run multiple times
    @param decayStringForDirectionOnlyParticles DecayString specifying the particles
        to use only direction information in the fit
    @param decayStringForAlternateMassParticles DecayString specifying the particles
        where an alternate mass hypothesis is used
    @param alternateMassHypos list of pdg values (or particle names) for particles where
        different mass hypothesis is used in the fit
    @param decayStringForNeutronVsAntiNeutron DecayString specifying the charged particle
        used to tag whether n or nbar. When tag particle has negative charge, PDG sign of n/nbar
        is flipped from default given in alternateMassHypos
    @param path         modules are added to this path
    """

    orca = register_module('ParticleKinematicFitter')
    orca.set_name('ParticleKinematicFitter_' + list_name)
    orca.param('debugFitter', False)
    orca.param('orcaTracer', 'None')
    orca.param('orcaFitterEngine', fitterEngine)
    orca.param('orcaConstraint', constraint)
    orca.param('invMass', invMass)
    orca.param('listName', list_name)
    orca.param('updateDaughters', daughtersUpdate)
    orca.param('addUnmeasuredPhoton', False)
    orca.param('variablePrefix', variablePrefix)
    orca.param('decayStringForDirectionOnlyParticles', decayStringForDirectionOnlyParticles)
    orca.param('decayStringForAlternateMassParticles', decayStringForAlternateMassParticles)
    orca.param('decayStringForNeutronVsAntiNeutron', decayStringForNeutronVsAntiNeutron)
    orca.param('alternateMassHypos', pdg.from_names(alternateMassHypos))
    path.add_module(orca)


if __name__ == '__main__':
    from basf2.utils import pretty_print_module
    pretty_print_module(__name__, "kinfit")
