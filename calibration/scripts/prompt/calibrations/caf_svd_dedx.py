#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
'''
Script to perform the SVD dE/dx calibration
'''
import ROOT
from prompt import CalibrationSettings, INPUT_DATA_FILTERS
import basf2 as b2
from ROOT.Belle2 import SVDdEdxCalibrationAlgorithm

import modularAnalysis as ma
import vertex as vx

ROOT.gROOT.SetBatch(True)


settings = CalibrationSettings(
    name="caf_svd_dedx",
    expert_username="lisovsky",
    description=__doc__,
    input_data_formats=["cdst"],
    input_data_names=["hadron_calib"],
    input_data_filters={"hadron_calib": [INPUT_DATA_FILTERS["Data Tag"]["hadron_calib"],
                                         INPUT_DATA_FILTERS["Beam Energy"]["4S"],
                                         INPUT_DATA_FILTERS["Beam Energy"]["Continuum"],
                                         INPUT_DATA_FILTERS["Run Type"]["physics"],
                                         INPUT_DATA_FILTERS["Magnet"]["On"]]},

    expert_config={
        "MaxFilesPerRun": 5,
        "MinEvtsPerFile": 1,
        "MaxEvtsPerRun": 1.e6,
        "MinEvtsPerTree": 100,
        "NBinsP": 69,
        "NBinsdEdx": 100,
        "dedxCutoff": 5.e6
        },
    depends_on=[])


def get_calibrations(input_data, **kwargs):
    """
    Parameters:
      input_data (dict): Should contain every name from the 'input_data_names' variable as a key.
        Each value is a dictionary with {"/path/to/file_e1_r5.root": IoV(1,5,1,5), ...}. Useful for
        assigning to calibration.files_to_iov

      **kwargs: Configuration options to be sent in. Since this may change we use kwargs as a way to help prevent
        backwards compatibility problems. But you could use the correct arguments in b2caf-prompt-run for this
        release explicitly if you want to.

        Currently only kwargs["requested_iov"] and kwargs["expert_config"] are used.

        "requested_iov" is the IoV range of the bucket and your payloads should correspond to this range.
        However your highest payload IoV should be open ended e.g. IoV(3,4,-1,-1)

        "expert_config" is the input configuration. It takes default values from your `CalibrationSettings` but these are
        overwritten by values from the 'expert_config' key in your input `caf_config.json` file when running ``b2caf-prompt-run``.

    Returns:
      list(caf.framework.Calibration): All of the calibration objects we want to assign to the CAF process
    """
    import basf2
    # Set up config options

    # In this script we want to use one sources of input data.
    # Get the input files  from the input_data variable
    file_to_iov_hadron_calib = input_data["hadron_calib"]

    expert_config = kwargs.get("expert_config")
    max_files_per_run = expert_config["MaxFilesPerRun"]

    # If you are using Raw data there's a chance that input files could have zero events.
    # This causes a B2FATAL in basf2 RootInput so the collector job will fail.
    # Currently we don't have a good way of filtering this on the automated side, so we can check here.
    min_events_per_file = expert_config["MinEvtsPerFile"]

    from prompt.utils import filter_by_max_files_per_run

    reduced_file_to_iov_hadron_calib = filter_by_max_files_per_run(file_to_iov_hadron_calib, max_files_per_run, min_events_per_file)
    input_files_hadron_calib = list(reduced_file_to_iov_hadron_calib.keys())
    basf2.B2INFO(f"Total number of files actually used as input = {len(input_files_hadron_calib)}")

    # Get the overall IoV we our process should cover. Includes the end values that we may want to ignore since our output
    # IoV should be open ended. We could also use this as part of the input data selection in some way.
    requested_iov = kwargs.get("requested_iov", None)

    from caf.utils import IoV
    # The actual value our output IoV payload should have. Notice that we've set it open ended.
    output_iov = IoV(requested_iov.exp_low, requested_iov.run_low, -1, -1)

    ###################################################
    # Algorithm setup

    algo = SVDdEdxCalibrationAlgorithm()
    algo.setMonitoringPlots(True)
    algo.setNumPBins(expert_config['NBinsP'])
    algo.setNumDEdxBins(expert_config['NBinsdEdx'])
    algo.setDEdxCutoff(expert_config['dedxCutoff'])
    algo.setMinEvtsPerTree(expert_config['MinEvtsPerTree'])

    ###################################################
    # Calibration setup

    from caf.framework import Calibration

    rec_path = b2.Path()
    rec_path.add_module('RootInput')

    # Fill particle lists
    ma.fillParticleList("pi+:all", "", path=rec_path)
    ma.fillParticleList("pi+:lam", "nCDCHits>0", path=rec_path)  # pi without track quality for reconstructing lambda
    ma.fillParticleList("pi+:cut", "abs(dr)<0.5 and abs(dz)<2 and pValue > 0.00001 and nSVDHits > 1", path=rec_path)

    ma.fillParticleList('K-:cut', cut='abs(dr)<0.5 and abs(dz)<2 and pValue > 0.00001 and nSVDHits > 1', path=rec_path)  # kaon
    ma.fillParticleList('e+:cut', cut='nSVDHits > 0', path=rec_path)  # electron
    ma.fillParticleList('p+:lam', cut='nCDCHits>0 and nSVDHits > 0', path=rec_path)  # proton

    # ----------------------------------------------------------------------------
    # Reconstruct D*(D0->K-pi+)pi+ and cc.
    ma.reconstructDecay(decayString='D0:kpi -> K-:cut pi+:cut', cut='1.7 < M < 2.', path=rec_path)
    ma.reconstructDecay(
        decayString='D*+:myDstar -> D0:kpi pi+:all',
        cut='1.95 < M <2.05 and massDifference(0) < 0.16',
        path=rec_path)

    # Reconstruct Lambda->p+pi- and cc.
    ma.reconstructDecay('Lambda0:myLambda -> p+:lam pi-:lam', '1.1 < M < 1.3', path=rec_path)

    # Reconstruct gamma->e+e- (photon conversion)
    ma.reconstructDecay('gamma:my_gamma -> e+:cut e-:cut', '0.0 < M < 0.5', path=rec_path)

    # ----------------------------------------------------------------------------
    # vertex fits
    vx.treeFit(list_name='D*+:myDstar', conf_level=0, ipConstraint=True, updateAllDaughters=True, path=rec_path)
    vx.treeFit(list_name='Lambda0:myLambda', conf_level=0, ipConstraint=True, updateAllDaughters=True, path=rec_path)
    vx.treeFit(list_name='gamma:my_gamma', conf_level=0, path=rec_path)

    # ----------------------------------------------------------------------------
    # Selections on Lambda

    ma.cutAndCopyList(
        outputListName='Lambda0:cut',
        inputListName='Lambda0:myLambda',
        cut=" ".join([
            "1.10 < InvM < 1.13 and chiProb > 0.001 and distance>1.0 and ",
            "formula(daughter(0,p))>formula(daughter(1,p)) and convertedPhotonInvariantMass(0,1)>0.02 and ",
            "[[formula((((daughter(0, px)**2+daughter(0, py)**2+daughter(0, pz)**2 + 0.13957**2)**0.5+",
            "daughter(1, E))*((daughter(0, px)**2+daughter(0, py)**2+daughter(0, pz)**2 + 0.13957**2)**0.5+",
            "daughter(1, E))-(daughter(0, px)+daughter(1, px))*(daughter(0, px)+daughter(1, px))-(daughter(0, py)+",
            "daughter(1, py))*(daughter(0, py)+daughter(1, py))-(daughter(0, pz)+daughter(1, pz))*(daughter(0, pz)+",
            "daughter(1, pz)))**0.5)<0.488]",
            "or [formula((((daughter(0, px)**2+daughter(0, py)**2+daughter(0, pz)**2 + 0.13957**2)**0.5+",
            "daughter(1, E))*((daughter(0, px)**2+daughter(0, py)**2+daughter(0, pz)**2 + 0.13957**2)**0.5+",
            "daughter(1, E))-(daughter(0, px)+daughter(1, px))*(daughter(0, px)+daughter(1, px))-(daughter(0, py)+",
            "daughter(1, py))*(daughter(0, py)+daughter(1, py))-(daughter(0, pz)+daughter(1, pz))*(daughter(0, pz)+",
            "daughter(1, pz)))**0.5)>0.513]]"
        ]),
        path=rec_path)

    # ----------------------------------------------------------------------------
    # Selections on Dstar

    ma.cutAndCopyList(
        outputListName='D*+:cut',
        inputListName='D*+:myDstar',
        cut='massDifference(0) < 0.151 and 1.85 < daughter(0, InvM) < 1.88 and 1.95 < InvM < 2.05',
        path=rec_path)

    # ----------------------------------------------------------------------------
    # Selections on gamma
    ma.buildEventShape(inputListNames='gamma:my_gamma', path=rec_path)
    ma.cutAndCopyList(
        outputListName='gamma:cut',
        inputListName='gamma:my_gamma',
        cut=" ".join(['chiProb > 0.001 and 1 < dr < 12 and InvM < 0.01',
                      'and convertedPhotonInvariantMass(0,1) < 0.005',
                      'and -0.05<convertedPhotonDelR(0,1)<0.15',
                      'and -0.05<convertedPhotonDelZ(0,1)<0.05'
                      ]),
        path=rec_path)

    cal_test = Calibration("SVDdEdxCalibration",
                           collector="SVDdEdxCollector",
                           algorithms=[algo],
                           input_files=input_files_hadron_calib,
                           pre_collector_path=rec_path)
    # Do this for the default AlgorithmStrategy to force the output payload IoV
    # It may be different if you are using another strategy like SequentialRunByRun
    for algorithm in cal_test.algorithms:
        algorithm.params = {"apply_iov": output_iov}

    # Most other options like database chain and backend args will be overwritten by b2caf-prompt-run.
    # So we don't bother setting them.

    # You must return all calibrations you want to run in the prompt process, even if it's only one
    return [cal_test]

##############################
