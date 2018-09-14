#!/usr/bin/env bash

OS=$(uname | grep '_NT')

if [ "$OS" != "" ]; then
    gen="'Visual Studio 15 Win64'"
    cflags=""
    cxxflags=""
    export AKMALLOC_LINK_STATIC=1
else
    gen=""
    cflags="-g"
    cxxflags="-std=c++11 -Wno-unused-variable -pthread"
    export AKMALLOC_LIBRARY=1
fi

env CFLAGS="$cflags" CXXFLAGS="$cxxflags" VERBOSE=1 \
    bash -c "tools/build $1 $gen"
