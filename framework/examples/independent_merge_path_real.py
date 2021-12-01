#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# @cond

import basf2
from ROOT import Belle2


class CheckData(basf2.Module):

    """check output of CreateData"""

    def initialize(self):
        """reimplementation"""

        self.tracks = Belle2.PyStoreArray('Tracks')
        self.clusters = Belle2.PyStoreArray('ECLClusters')

    def event(self):
        """reimplementation"""

        print(self.name())
        print("nTracks: " + str(self.tracks.getEntries()))
        print("nECLClusters: " + str(self.clusters.getEntries()))

        for track in self.tracks:
            for cluster in track.getRelationsTo('ECLClusters'):
                print(track.getArrayIndex(), '->', cluster.getArrayIndex())


class CheckIndices(basf2.Module):

    """check output of CreateData"""

    def initialize(self):
        """reimplementation"""

        self.tracks = Belle2.PyStoreArray('Tracks')
        self.v0s = Belle2.PyStoreArray('V0s')

    def event(self):
        """reimplementation"""

        print(self.name())
        print("Tracks")
        for track in self.tracks:
            for tfr in track.getTrackFitResults():
                print(track.getArrayIndex(), '->', tfr.second.getArrayIndex())
        print("V0s")
        for v0 in self.v0s:
            print('t', v0.getArrayIndex(), '->', v0.getTracks().first.getArrayIndex(), '/', v0.getTracks().second.getArrayIndex())
            print(
                'f',
                v0.getArrayIndex(),
                '->',
                v0.getTrackFitResults().first.getArrayIndex(),
                '/',
                v0.getTrackFitResults().second.getArrayIndex())


main = basf2.Path()
indep = basf2.Path()

# input
input1 = basf2.register_module('RootInput')
input1.param('inputFileName', '/nfs/dust/belle2/user/kurzsimo/testSample/file1_10evts.root')
main.add_module(input1).set_name("input1")

main.add_module(CheckData()).set_name("checkdata_main")

# and the other input
input2 = basf2.register_module('RootInput')
input2.param('inputFileName', '/nfs/dust/belle2/user/kurzsimo/testSample/file2_10evts.root')
indep.add_module(input2).set_name("input2")

indep.add_module(CheckData()).set_name("checkdata_indep")

# merge it!
# Use merge_back_event=['ALL'] to merge everything
# NOTE: StoreArrays have to be merged before their Relations
main.add_independent_merge_path(
    indep,
    merge_back_event=[
        'ALL'
    ])
# main.add_independent_merge_path(
# indep,
# merge_back_event=[
# 'ECLClusters',
# 'Tracks',
# 'MCParticles',
# 'TracksToECLClusters',
# 'TracksToMCParticles',
# 'ECLClustersToMCParticles',
# 'ECLClustersToECLClusters'
# ])
#

main.add_module(CheckData()).set_name("checkdata_merged")

main.add_module(CheckIndices()).set_name("checkindices")
main.add_module('FixMergedObjects')
main.add_module(CheckIndices()).set_name("checkindices_fixed")

# output
output = basf2.register_module('RootOutput')
output.param('outputFileName', '/nfs/dust/belle2/user/kurzsimo/testSample/merged_10evts.root')
main.add_module(output)

# progress
main.add_module('Progress')

basf2.print_path(main)
basf2.process(main)

print(basf2.statistics)

# @endcond
