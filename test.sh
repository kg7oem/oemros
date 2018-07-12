#!/usr/bin/env bash

set -e
set -x

BUILD=./build.sh

export VERBOSE=1

"$BUILD"
CXX=g++ "$BUILD"
CXX=clang++ "$BUILD"

echo tests pass
