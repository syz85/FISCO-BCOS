#!/usr/bin/env bash
rm -rf cmake-build-aarch64
mkdir -p cmake-build-aarch64
cd cmake-build-aarch64
cmake -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ -DARCHITECTURE=aarch64 ..