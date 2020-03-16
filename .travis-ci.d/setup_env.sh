#!/bin/bash

source /cvmfs/dune.opensciencegrid.org/products/dune/setup_dune.sh

setup dk2nu        v01_05_01b    -q e15:prof
setup genie        v2_12_10c     -q e15:prof
setup genie_xsec   v2_12_10      -q DefaultPlusValenciaMEC
setup genie_phyopt v2_12_10      -q dkcharmtau
setup geant4       v4_10_3_p01b  -q e15:prof
setup cmake        v3_14_3
