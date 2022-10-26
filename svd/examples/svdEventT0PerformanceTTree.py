#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##################################################################################
#
# Example Script to use SVDEventT0PerformanceTTree and OverlapResidual Modules
# on simulation or real data
#
# Use: basf2 -i /pnfs/desy.de/belle/belle2/TMP/belle/Raw/e0016/physics/r00610/sub00/physics.0016.00610.HLT1.f00000.root
#      svd/examples/svdEventT0PerformanceTTree.py
#      basf2 -i /pnfs/desy.de/belle/belle2/TMP/belle/Raw/e0024/physics/r01726/sub01/physics.0024.01726.HLT1.f00000.root
#      svd/examples/svdEventT0PerformanceTTree.py
#
#
###################################################################################

import basf2 as b2
from basf2 import conditions as b2conditions
import rawdata as raw
import tracking as trk
import simulation as sim
import glob
# import sys
# import argparse
# import os

# useSimulation = True
useSimulation = False

# set this string to identify the output rootfiles
ftag = "_test"

main = b2.create_path()

b2.set_random_seed(1)

if useSimulation:
    # options for simulation:
    # expList = [1003]
    expList = [0]
    numEvents = 20
    bkgFiles = glob.glob('/sw/belle2/bkg/*.root')  # Phase3 background
    bkgFiles = None  # uncomment to remove  background
    simulateJitter = False
    ROIfinding = False
    MCTracking = False
    eventinfosetter = b2.register_module('EventInfoSetter')
    eventinfosetter.param('expList', expList)
    eventinfosetter.param('runList', [0])
    eventinfosetter.param('evtNumList', [numEvents])
    main.add_module(eventinfosetter)
    # main.add_module('EventInfoPrinter')
    main.add_module('EvtGenInput')

    sim.add_simulation(
        main,
        bkgfiles=bkgFiles,
        forceSetPXDDataReduction=True,
        usePXDDataReduction=ROIfinding,
        simulateT0jitter=simulateJitter)
else:
    # setup database
    b2conditions.reset()
    b2conditions.override_globaltags()
    b2conditions.globaltags = ["online"]
    b2conditions.prepend_globaltag("staging_online")
    # b2conditions.append_globaltag("svd_data_rel7")
    # b2conditions.append_globaltag("patch_main_release-07_noTOP")
    b2conditions.prepend_globaltag("patch_main_release-07")

    MCTracking = False

    # input root files
    # main.add_module('RootInput', branchNames=['RawPXDs', 'RawSVDs', 'RawCDCs', 'RawECLs'])
    # raw.add_unpackers(main, components=['PXD', 'SVD', 'CDC', 'ECL'])

    main.add_module('RootInput', entrySequences=['0:100'])

    main.add_module("Gearbox")
    main.add_module('Geometry', useDB=True)

    raw.add_unpackers(main)

    # change ZS to 5
    # for moda in main.modules():
    #    if moda.name() == 'SVDUnpacker':
    #        moda.param("svdShaperDigitListName", "SVDShaperDigitsZS3")
    # main.add_module("SVDZeroSuppressionEmulator",SNthreshold=5,ShaperDigits="SVDShaperDigitsZS3",ShaperDigitsIN="SVDShaperDigits")

# now do reconstruction:
trk.add_tracking_reconstruction(
    main,
    mcTrackFinding=MCTracking,
    trackFitHypotheses=[211])  # ,
#    skipHitPreparerAdding=True)

'''
# skim mu+mu- events:
ma.applyEventCuts("nTracks ==2", path=main)

mySelection = 'pt>1.0 and abs(dz)<0.5 and dr<0.4'
ma.fillParticleList('mu+:DQM', mySelection, path=main)
ma.reconstructDecay('Upsilon(4S):IPDQM -> mu+:DQM mu-:DQM', '10<M<11', path=main)

skimfilter = b2.register_module('SkimFilter')
skimfilter.set_name('SkimFilter_MUMU')
skimfilter.param('particleLists', ['Upsilon(4S):IPDQM'])
main.add_module(skimfilter)
filter_path = b2.create_path()
skimfilter.if_value('=1', filter_path, b2.AfterConditionPath.CONTINUE)
'''

# fill TTrees
# main.add_module('SVDPerformanceTTree', outputFileName="SVDPerformanceTree"+str(tag)+".root")
main.add_module('SVDEventT0PerformanceTTree', outputFileName="SVDEventT0PerformanceTTree"+str(ftag)+".root")

# # write everything
# main.add_module('OverlapResiduals', ExpertLevel=True)

# main.add_module('RootOutput')
main.add_module('Progress')

b2.print_path(main)

b2.process(main)

print(b2.statistics)
