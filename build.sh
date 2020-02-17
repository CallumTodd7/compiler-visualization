#!/bin/bash

case $1 in
  Debug|"")
    mkdir -p cmake-build-debug
    cd cmake-build-debug
    cmake -DCMAKE_BUILD_TYPE=Debug -G "CodeBlocks - Unix Makefiles" .. && make
    ;;
  Release)
    mkdir -p cmake-build-release
    cd cmake-build-release
    cmake -DCMAKE_BUILD_TYPE=Release -G "CodeBlocks - Unix Makefiles" .. && make
    ;;
  *)
    echo "Unknown build type. Must be: Release, RelWithDebInfo, Debug."
esac
