#!/bin/bash
set -xe

mkdir build
cd build
cmake -GNinja -DMLIR_ROOT_DIR=/local/llvm -C ../cmake/caches/gc-dev.cmake -DOMTALK_SPLIT_DEBUG=OFF ..
ninja
ctest --output-on-failure
