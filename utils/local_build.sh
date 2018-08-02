#!/bin/bash

[[ -d build ]] && rm -r build

mkdir build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE="Debug" -DENABLE_NLS=true ../
scan-build ninja
