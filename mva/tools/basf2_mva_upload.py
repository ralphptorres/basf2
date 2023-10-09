#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2_mva

import argparse


class Once(argparse.Action):
    def __init__(self, *args, **kwargs):
        super(Once, self).__init__(*args, **kwargs)
        self._count = 0

    def __call__(self, parser, namespace, values, option_string=None):
        # print('{n} {v} {o}'.format(n=namespace, v=values, o=option_string))
        if self._count != 0:
            msg = '{o} can only be specified once'.format(o=option_string)
            raise argparse.ArgumentError(None, msg)
        self._count = 1
        setattr(namespace, self.dest, values)


def get_argument_parser() -> argparse.ArgumentParser:
    """ Parses the command line options of this tool """
    parser = argparse.ArgumentParser()
    parser.add_argument('--identifier', dest='identifier', type=str, required=True,
                        help='Identifier produced by basf2_mva_teacher')
    parser.add_argument('--db_identifier', dest='db_identifier', type=str, required=True,
                        help='New database identifier for the method')
    parser.add_argument('--begin_experiment', dest='begin_experiment', type=int, required=False, action=Once,
                        help='First experiment for which the weightfile is valid', default=0)
    parser.add_argument('--end_experiment', dest='end_experiment', type=int, required=False, action=Once,
                        help='Last experiment for which the weightfile is valid', default=0)
    parser.add_argument('--begin_run', dest='begin_run', type=int, required=False, action=Once,
                        help='First run for which the weightfile is valid', default=-1)
    parser.add_argument('--end_run', dest='end_run', type=int, required=False, action=Once,
                        help='Last run for which the weightfile is valid', default=-1)
    return parser


if __name__ == '__main__':

    parser = get_argument_parser()
    args = parser.parse_args()
    basf2_mva.upload(args.identifier, args.db_identifier, args.begin_experiment, args.begin_run,
                     args.end_experiment, args.end_run)
