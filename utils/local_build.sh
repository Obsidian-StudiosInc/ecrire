#!/bin/bash

[[ -d build ]] && rm -r build

mkdir build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE="Debug" ../
cd ../
ninja -C build
