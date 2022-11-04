#!/usr/bin/env bash
mkdir -p build-result
cd build-result
cmake -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ -DARCHITECTURE=aarch64 ..