#!/usr/bin/env bash

OS=$(uname | grep '_NT')

if [ "$OS" != "" ]; then
    gen="'Visual Studio 15 Win64'"
    cflags=""
    cxxflags=""
else
    gen=""
    cflags="-g"
    cxxflags="-std=c++11 -Wno-unused-variable -pthread"
fi

env CFLAGS="$cflags" CXXFLAGS="$cxxflags" \
    VERBOSE=1 AKMALLOC_LINK_STATIC=1 \
    bash -c "tools/build $1 $gen"
