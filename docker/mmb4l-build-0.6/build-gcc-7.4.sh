#!/bin/bash

mkdir -p build-release-gcc-7.4
cd build-release-gcc-7.4
cmake -DCMAKE_BUILD_TYPE=Release ..
make

export version="0.6.0"
export arch=`uname -m`
export glibc_version="2.17"
export distrib=mmb4l-${version}-${arch}-glibc-${glibc_version}

mkdir -p ${distrib}
cp mmbasic ${distrib}
cp ../resources/mmbasic.nanorc ${distrib}
cp ../resources/mmbasic.syntax.nanorc ${distrib}
cp ../ChangeLog ${distrib}
cp ../LICENSE* ${distrib}
cp ../README.md ${distrib}
tar -cvzf ${distrib}.tgz ${distrib}
