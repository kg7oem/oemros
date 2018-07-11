#!/usr/bin/env bash

set -e
set -x

pushd src
rm -rf build/
mkdir build/

pushd build
cmake ..
time make -j4
