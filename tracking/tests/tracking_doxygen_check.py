#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Perform code doxygen checks for every commit to the tracking package.
"""

import re
from b2test_utils import check_error_free, is_ci, skip_test


if is_ci():
    skip_test("The test is too slow for running on our pipeline")

if __name__ == "__main__":
    ignoreme = 'IGNORE_NOTHING'
    check_error_free("b2code-doxygen-warnings", "doxygen", "tracking",
                     lambda x: re.findall(ignoreme, x) or x == "'")
