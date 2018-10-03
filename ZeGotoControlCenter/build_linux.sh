#!/bin/sh
if [ ! -d build ]; then
  mkdir build
fi
cd build
cmake -G "Unix Makefiles" -D "Qt5_DIR=~/Qt/5.5/gcc_64/lib/cmake/Qt5" ..
make
make package

