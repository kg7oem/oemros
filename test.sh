#!/usr/bin/env bash

set -e
set -x

BUILD=./build.sh

"$BUILD"
CXX=g++ "$BUILD"
CXX=clang++ "$BUILD"

echo tests pass
