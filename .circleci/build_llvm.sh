#!/bin/bash

set -xe

#TODO this should be set somewhere else
set LLVM_INSTALL_DIR=$HOME/llvm
sudo apt-get update -y
sudo apt-get install -y g++ gcc ninja-build lld-8 cmake 
#$LLVM_BIN_DIR
if [ -e $LLVM_INSTALL_DIR/lib/cmake/mlir/AddMLIR.cmake ] ; then
	echo "Could not find cached llvm, rebuilding"
	git submodule update --init --depth 1 external/llvm-project
	LLVM_SRC_DIR=$PWD/external/llvm-project/llvm
	LLVM_BUILD_DIR=$PWD/build/llvm-project
	mkdir $LLVM_INSTALL_DIR

	pushd $LLVM_BUILD_DIR
	cmake -G Ninja \
	 -DCMAKE_INSTALL_PREFIX=$LLVM_INSTALL_DIR \
	 -DLLVM_ENABLE_MODULES=ON \
	 -DLLVM_BUILD_TOOLS=OFF \
	 -DLLVM_BUILD_EXAMPLES=OFF \
	 -DLLVM_BUILD_TESTS=OFF \
	 -DLLVM_BUILD_BENCHMARKS=OFF \
	 -DLLVM_ENABLE_PROJECTS=mlir \
	 -DLLVM_USE_LINKER=lld \
	 -DLLVM_OPTIMIZED_TABLEGEN=ON \
	 -DLLVM_TARGETS_TO_BUILD=X86 \
	 $LLVM_SRC_DIR
	ninja install
fi
