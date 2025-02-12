#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
<header>
  <contact>software-tracking@belle2.org</contact>
  <input>EvtGenSimNoBkg.root</input>
  <output>CombinedSVDTrackingValidation.root</output>
  <description>
  This module validates that the combined VXDTF2 and SVDHoughTracking is capable of reconstructing tracks in Y(4S) runs.
  </description>
</header>
"""

from tracking.validation.run import TrackingValidationRun
from tracking.path_utils import add_hit_preparation_modules, add_svd_standalone_tracking
import logging
import basf2

VALIDATION_OUTPUT_FILE = 'CombinedSVDTrackingValidation.root'
N_EVENTS = 1000
ACTIVE = False


class CombinedSVDTrackingValidation(TrackingValidationRun):
    """
    Validation class for the four 4-SVD Layer tracking
    """
    #: the number of events to process
    n_events = N_EVENTS
    #: Generator to be used in the simulation (-so)
    generator_module = 'generic'
    #: root input file to use, generated by central validation script
    root_input_file = '../EvtGenSimNoBkg.root'
    #: use full detector for validation
    components = None

    @staticmethod
    def finder_module(path):
        """Add the combined SVD standalone track finders and related modules to the basf2 path"""
        add_hit_preparation_modules(path, components=["SVD"])
        add_svd_standalone_tracking(path, reco_tracks="RecoTracks", svd_standalone_mode="VXDTF2_and_SVDHough")

    #: use only the svd hits when computing efficiencies
    tracking_coverage = {
        'WhichParticles': ['SVD'],  # Include all particles seen in the SVD detector, also secondaries
        'UsePXDHits': False,
        'UseSVDHits': True,
        'UseCDCHits': False,
    }

    #: perform fit after track finding
    fit_tracks = True
    #: plot pull distributions
    pulls = True
    #: create expert-level histograms
    use_expert_folder = True
    #: Include resolution information in the validation output
    resolution = True
    #: Use the fit information in validation
    use_fit_information = True
    #: output file of plots
    output_file_name = VALIDATION_OUTPUT_FILE
    #: define empty list of non expert parameters so that no shifter plots are created (to revert just remove following line)
    non_expert_parameters = []


def main():
    """
    create SVD validation class and execute
    """
    basf2.set_random_seed(1337)
    validation_run = CombinedSVDTrackingValidation()
    validation_run.configure_and_execute_from_commandline()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    if ACTIVE:
        main()
    else:
        print("This validation deactivated and thus basf2 is not executed.\n"
              "If you want to run this validation, please set the 'ACTIVE' flag above to 'True'.\n"
              "Exiting.")
