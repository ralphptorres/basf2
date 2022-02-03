#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2_mva

if __name__ == "__main__":
    from basf2 import conditions
    import ROOT  # noqa
    # NOTE: do not use testing payloads in production! Any results obtained like this WILL NOT BE PUBLISHED
    conditions.testing_payloads = [
        'localdb/database.txt'
    ]

    variables = ['M', 'p', 'pt', 'pz', 'phi',
                 'daughter(0, p)', 'daughter(0, pz)', 'daughter(0, pt)', 'daughter(0, phi)',
                 'daughter(1, p)', 'daughter(1, pz)', 'daughter(1, pt)', 'daughter(1, phi)',
                 'daughter(2, p)', 'daughter(2, pz)', 'daughter(2, pt)', 'daughter(2, phi)',
                 'chiProb', 'dr', 'dz', 'dphi',
                 'daughter(0, dr)', 'daughter(1, dr)', 'daughter(0, dz)', 'daughter(1, dz)',
                 'daughter(0, dphi)', 'daughter(1, dphi)',
                 'daughter(0, chiProb)', 'daughter(1, chiProb)', 'daughter(2, chiProb)', 'daughter(2, M)',
                 'daughter(0, atcPIDBelle(3,2))', 'daughter(1, atcPIDBelle(3,2))',
                 'daughterAngle(0, 1)', 'daughterAngle(0, 2)', 'daughterAngle(1, 2)',
                 'daughter(2, daughter(0, E))', 'daughter(2, daughter(1, E))',
                 'daughter(2, daughter(0, clusterLAT))', 'daughter(2, daughter(1, clusterLAT))',
                 'daughter(2, daughter(0, clusterHighestE))', 'daughter(2, daughter(1, clusterHighestE))',
                 'daughter(2, daughter(0, clusterNHits))', 'daughter(2, daughter(1, clusterNHits))',
                 'daughter(2, daughter(0, clusterE9E25))', 'daughter(2, daughter(1, clusterE9E25))',
                 'daughter(2, daughter(0, minC2TDist))', 'daughter(2, daughter(1, minC2TDist))',
                 'daughterInvM(1, 2)']

    # Perform an sPlot training
    general_options = ROOT.Belle2.MVA.GeneralOptions()
    general_options.m_datafiles = basf2_mva.vector("train_mc.root")
    general_options.m_identifier = "MVAFull"
    general_options.m_treename = "tree"
    general_options.m_variables = basf2_mva.vector(*variables)
    general_options.m_target_variable = "isSignal"

    fastbdt_options = ROOT.Belle2.MVA.FastBDTOptions()
    # SPlot is more stable if one doesn't use the randRatio
    # FastBDT has a special sPlot mode, but which isn't implemented yet in the mva package
    fastbdt_options.m_nTrees = 100
    fastbdt_options.m_randRatio = 1.0
    ROOT.Belle2.MVA.teacher(general_options, fastbdt_options)

    general_options.m_identifier = "MVAOrdinary"
    general_options.m_variables = basf2_mva.vector(*variables[1:])
    ROOT.Belle2.MVA.teacher(general_options, fastbdt_options)

    meta_options = ROOT.Belle2.MVA.MetaOptions()
    meta_options.m_use_splot = True
    meta_options.m_splot_variable = "M"
    # SPlot training assumes that the datafile given to the general options contains only data
    # It requires an additional file with MC information from which it can extract the distribution
    # of the discriminating variable (in this case M).
    # Here we use the same file
    general_options.m_datafiles = basf2_mva.vector("train_data.root")
    meta_options.m_splot_mc_files = basf2_mva.vector("train_mc.root")

    # First we do an ordinary sPlot training
    general_options.m_identifier = "MVASPlot"
    meta_options.m_splot_combined = False
    meta_options.m_splot_boosted = False
    ROOT.Belle2.MVA.teacher(general_options, fastbdt_options, meta_options)

    # Now we combine the sPlot training with a PDF classifier for M, in one step
    general_options.m_identifier = "MVASPlotCombined"
    meta_options.m_splot_combined = True
    meta_options.m_splot_boosted = False
    ROOT.Belle2.MVA.teacher(general_options, fastbdt_options, meta_options)

    # Now we use a boosted sPlot training
    general_options.m_identifier = "MVASPlotBoosted"
    meta_options.m_splot_combined = False
    meta_options.m_splot_boosted = True
    ROOT.Belle2.MVA.teacher(general_options, fastbdt_options, meta_options)

    # And finally a boosted and combined training
    general_options.m_identifier = "MVASPlotCombinedBoosted"
    meta_options.m_splot_combined = True
    meta_options.m_splot_boosted = True
    ROOT.Belle2.MVA.teacher(general_options, fastbdt_options, meta_options)

    # Also do a training of only the pdf classifier
    pdf_options = ROOT.Belle2.MVA.PDFOptions()
    general_options.m_method = 'PDF'
    general_options.m_identifier = "MVAPdf"
    general_options.m_variables = basf2_mva.vector('M')
    ROOT.Belle2.MVA.teacher(general_options, pdf_options)

    # Apply the trained methods on data
    ROOT.Belle2.MVA.expert(basf2_mva.vector('MVAPdf', 'MVAFull', 'MVAOrdinary', 'MVASPlot',
                                            'MVASPlotCombined', 'MVASPlotBoosted', 'MVASPlotCombinedBoosted'),
                           basf2_mva.vector('train.root'), 'tree', 'expert.root')

    """
    path = b2.create_path()
    ma.inputMdstList('MC6', ['/storage/jbod/tkeck/MC6/evtgen-charged/sub00/mdst_0001*.root'], path=path)
    ma.fillParticleLists([('K-', 'kaonID > 0.5'), ('pi+', 'pionID > 0.5')], path=path)
    ma.reconstructDecay('D0 -> K- pi+', '1.8 < M < 1.9', path=path)
    vx.kFit('D0', 0.1, path=path)
    ma.applyCuts('D0', '1.8 < M < 1.9', path=path)
    ma.matchMCTruth('D0', path=path)

    path.add_module('MVAExpert', listNames=['D0'], extraInfoName='Pdf', identifier='MVAPdf')
    path.add_module('MVAExpert', listNames=['D0'], extraInfoName='Full', identifier='MVAFull')
    path.add_module('MVAExpert', listNames=['D0'], extraInfoName='Ordinary', identifier='MVAOrdinary')
    path.add_module('MVAExpert', listNames=['D0'], extraInfoName='SPlot', identifier='MVASPlot')
    path.add_module('MVAExpert', listNames=['D0'], extraInfoName='SPlotCombined', identifier='MVASPlotCombined')
    path.add_module('MVAExpert', listNames=['D0'], extraInfoName='SPlotBoosted', identifier='MVASPlotBoosted')
    path.add_module('MVAExpert', listNames=['D0'], extraInfoName='SPlotCombinedBoosted', identifier='MVASPlotCombinedBoosted')
    ma.variablesToNtuple('D0', ['isSignal', 'extraInfo(Pdf)', 'extraInfo(Full)', 'extraInfo(Ordinary)', 'extraInfo(SPlot)',
                             'extraInfo(SPlotCombined)', 'extraInfo(SPlotBoosted)', 'extraInfo(SPlotCombinedBoosted)'], path=path)
    b2.process(path)
    """
