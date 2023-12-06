#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#################################################################
#                                                               #
#    script to simulate 1000 charged-muon single-track events   #
#    using the ParticleGun for ext and muid validation. (No     #
#    background hits overlaid.)                                 #
#                                                               #
#################################################################

"""
<header>
    <output>muon-ExtMuidValidation.root</output>
    <contact>piilonen@vt.edu</contact>
    <description>Create events with 1 muon track for ext/muid validation.</description>
</header>
"""

import glob
import basf2 as b2
import os
from simulation import add_simulation
from reconstruction import add_reconstruction


def run():
    """
    Create muon sample for the ExtMuid validation.
    """
    b2.set_random_seed(123460)

    output_filename = '../muon-ExtMuidValidation.root'

    print(output_filename)

    path = b2.create_path()

    eventinfosetter = b2.register_module('EventInfoSetter')
    eventinfosetter.param('evtNumList', [1000])
    path.add_module(eventinfosetter)

    pgun = b2.register_module('ParticleGun')
    param_pgun = {
        'pdgCodes': [-13, 13],
        'nTracks': 1,
        'varyNTracks': 0,
        'momentumGeneration': 'uniform',
        'momentumParams': [0.5, 5.0],
        'thetaGeneration': 'uniformCos',
        'thetaParams': [15., 150.],
        'phiGeneration': 'uniform',
        'phiParams': [0.0, 360.0],
        'vertexGeneration': 'fixed',
        'xVertexParams': [0.0],
        'yVertexParams': [0.0],
        'zVertexParams': [0.0],
    }
    pgun.param(param_pgun)
    path.add_module(pgun)

    # add simulation and reconstruction modules to the path
    if 'BELLE2_BACKGROUND_DIR' in os.environ:
        background_files = glob.glob(os.environ['BELLE2_BACKGROUND_DIR'] + '/*.root')
        add_simulation(path, bkgfiles=background_files)
    else:
        b2.B2FATAL('BELLE2_BACKGROUND_DIR is not set.')

    add_reconstruction(path)

    output = b2.register_module('RootOutput')
    output.param('outputFileName', output_filename)
    output.param('branchNames', ['MCParticles', 'ExtHits', 'KLMMuidLikelihoods', 'KLMHit2ds'])
    path.add_module(output)
    path.add_module('Progress')

    b2.process(path)
    print(b2.statistics)


if __name__ == '__main__':
    run()
