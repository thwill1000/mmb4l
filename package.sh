#!/bin/bash

export version="2021.01.00-a1"
export arch=`uname -m`
export distrib=mmb4l-${version}-${arch}

rm -fR build-distrib
mkdir -p build-distrib
cd build-distrib
cmake -DCMAKE_BUILD_TYPE=Release ..
make mmbasic
mkdir ${distrib}
cp mmbasic ${distrib}
cp ../resources/mmbasic.nanorc ${distrib}
cp ../resources/mmbasic.syntax.nanorc ${distrib}
cp ../ChangeLog ${distrib}
cp ../README.md ${distrib}
tar -cvzf ${distrib}.tgz ${distrib}

export mmb4l_dir=../../MMB4L
cp ${distrib}.tgz ${mmb4l_dir}/binaries
cp ../ChangeLog ${mmb4l_dir}
cp ../README.md ${mmb4l_dir}
cp ../examples/mandelbrot.bas ${mmb4l_dir}/examples
