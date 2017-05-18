#!/usr/bin/env bash
./build/test/tMallocPerf > new.log
./build/test/tMallocPerfRef > old.log
vimdiff old.log new.log
