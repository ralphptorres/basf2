#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Perform code quality cppchecks for every commit to the simulation package.
"""

import re
from b2test_utils import check_error_free
from b2test_utils import skip_test

skip_test("New cppcheck version in latest externals.")

if __name__ == "__main__":
    # Ignore the nofile .. [missingInclude] that is always at the end of cppcheck
    ignoreme = 'Cppcheck cannot find all the include files'
    check_error_free("b2code-cppcheck", "cppcheck", "simulation",
                     lambda x: re.findall(ignoreme, x) or x == "'")
