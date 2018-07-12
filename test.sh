#!/usr/bin/env bash

set -e
set -x

BUILD=./build.sh

export VERBOSE=1

"$BUILD" && valgrind --error-exitcode=101 src/build/oemros
CXX=g++ "$BUILD" && valgrind --error-exitcode=101 src/build/oemros
CXX=clang++ "$BUILD" && valgrind --error-exitcode=101 src/build/oemros

echo tests pass
