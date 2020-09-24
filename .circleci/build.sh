#!/bin/bash
set -xe

mkdir build
cd build
cmake -GNinja -DMLIR_ROOT_DIR=/local/llvm -C ../cmake/caches/all-dev.cmake ..
ninja
