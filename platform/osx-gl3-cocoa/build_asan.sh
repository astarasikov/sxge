#!/bin/bash

export LOCAL_LLVM_BUILD=/Users/alexander/Documents/workspace/builds/llvm/build
export ASAN_SYMBOLIZER_PATH=$(LOCAL_LLVM_BUILD)/bin/llvm-symbolizer
make clean
make -j8 WITH_SANITIZER=1
