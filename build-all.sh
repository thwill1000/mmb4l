#!/bin/bash

mkdir -p build-debug
cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
cd ..

mkdir -p build-release
cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ..
make
cd ..

mkdir -p build-coverage
cd build-coverage
cmake -DMMB4L_COVERAGE=1 ..
make
cd ..
