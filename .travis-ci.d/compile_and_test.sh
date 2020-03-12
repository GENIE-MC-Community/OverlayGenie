#!/bin/bash

source /Package/.travis-ci.d/setup_env.sh

cd /Package
mkdir build
cd build
cmake -GNinja -DCMAKE_CXX_FLAGS="-fdiagnostics-color=always" .. && \
ninja && \
ninja install
