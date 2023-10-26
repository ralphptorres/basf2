# -*- coding: utf-8 -*-

# @cond internal
# only public interfaces, to make package doc nicer. Also the only things imported by 'from grafei import *'
__all__ = ["graFEISaverModule", "FlagBDecayModule"]
# @endcond

from grafei.modules.graFEISaverModule import graFEISaverModule
from grafei.modules.FlagBDecayModule import FlagBDecayModule

print(r"""
                                                  ____  ____    _      ____  ____  _
                                                  |  _  |__|   /_\     |___  |___  |
                                                  |__|  |  \  /   \    |     |___  |

                                o
                               / \
                              /   \                             x-----x
                             /     \            |-----\         |\   /|        |-----\        0 1 3 3 5
                            o       \           |----- \        | \ / |        |----- \       1 0 3 3 5
                           / \       \          |----- /        |  x  |        |----- /       3 3 0 1 5
                          /   \       \         |-----/         | / \ |        |-----/        3 3 1 0 5
                         /     \       \                        |/   \|                       5 5 5 5 0
                        o       o       \                       x-----x
                       / \     / \       \
                      x   x   x   x       x

      Authors: Jacopo Cerasoli, Giulio Dujany, Lucas Martel, Corentin Santos 2022 - 2023
      More information in ---TO BE DEFINED---
      Code based on the work of Kahn et al 10.1088/2632-2153/ac8de0 (https://github.com/Helmholtz-AI-Energy/BaumBauen)
      Please consider citing both articles.
      """)
