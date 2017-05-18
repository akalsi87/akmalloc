#!/usr/bin/env bash
env CFLAGS='-g' CXXFLAGS='-std=c++11 -Wno-unused-variable -pthread' VERBOSE=1 AKMALLOC_LIBRARY=1 tools/build $1
