#!/bin/bash

rm -fR build-distrib
mkdir -p build-distrib
cd build-distrib
cmake -DCMAKE_BUILD_TYPE=Release ..
make mmbasic

# Extract version from the executable.
RAW_VERSION_STRING=`strings ./mmbasic | grep "@(#)"`
RAW_VERSION_ARRAY=($RAW_VERSION_STRING) # splits on spaces
MAJOR=`echo ${RAW_VERSION_ARRAY[2]:1} | cut -d '.' -f 1`
MINOR=`echo ${RAW_VERSION_ARRAY[2]} | cut -d '.' -f 2`
MICRO=`echo ${RAW_VERSION_ARRAY[2]} | cut -d '.' -f 3`
if (( MICRO > 300 )); then
    MICRO="."$((MICRO-300))
elif (( MICRO > 200 )); then
    MICRO="-rc."$((MICRO-200))
elif (( MICRO > 100 )); then
    MICRO="-beta."$((MICRO-100))
else
    MICRO="-alpha."${MICRO}
fi

VERSION=${MAJOR}"."${MINOR}${MICRO}
ARCH=`uname -m`
GLIBC=`../tools/glibc-check.sh -m ./mmbasic`
DISTRIB=mmb4l-${VERSION}-${ARCH}-glibc-${GLIBC}

mkdir ${DISTRIB}
cp mmbasic ${DISTRIB}
cp ../resources/mmbasic.nanorc ${DISTRIB}
cp ../resources/mmbasic.syntax.nanorc ${DISTRIB}
cp ../ChangeLog ${DISTRIB}
cp ../LICENSE* ${DISTRIB}
cp ../README.md ${DISTRIB}

tar -cvzf ${DISTRIB}.tgz ${DISTRIB}

