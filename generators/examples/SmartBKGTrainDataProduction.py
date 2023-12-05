##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import argparse
import os

from smartBKG import DECORR_LIST
from smartBKG.b2modules.NN_trainer_module import data_production

parser = argparse.ArgumentParser(
    description='''Generator variables saver.''',
    formatter_class=argparse.ArgumentDefaultsHelpFormatter,
)
parser.add_argument('-j', type=int, required=False, default=1,
                    help="Job ID for batch. Default for test", metavar="JOB_ID",
                    dest='job_id')
parser.add_argument('-f', type=str, required=False, default='./',
                    help="Input dir", metavar="IN_DIR",
                    dest='in_dir')
parser.add_argument('-o', type=str, required=False, default='./',
                    help="Out dir", metavar="OUT_DIR",
                    dest='out_dir')
parser.add_argument('-d', type=list, required=False, default=DECORR_LIST,
                    help="Event level variables to save", metavar="SAVE_VARS",
                    dest='save_vars')
parser.add_argument("--workers", type=int, default=False,
                    help="Number of workers (0 means no multiprocessing)")
args = parser.parse_args()

os.makedirs(args.out_dir, exist_ok=True)

if not args.save_vars:
    args.save_vars = None
data_prod = data_production(
    in_dir=args.in_dir, out_dir=args.out_dir,
    job_id=args.job_id, save_vars=args.save_vars
    )
data_prod.process()
data_prod.clean_up()
