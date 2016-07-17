#!/bin/bash

set -e

: ${n=13731707816857396218511477189051880183926672022487649441793167544537}

THIS_SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

mxe_version=1be3717e10d2fbd880d3d038f1c062897e184ef3
wget https://github.com/mxe/mxe/archive/$mxe_version.zip -O mxe.zip
unzip mxe.zip
mv mxe-$mxe_version mxe
cd mxe
make MXE_PLUGIN_DIRS="plugins/gcc6" MXE_TARGETS=i686-w64-mingw32.static jsoncpp cryptopp

mkdir build
cd build
cmake \
  -DCMAKE_TOOLCHAIN_FILE=$THIS_SCRIPT_DIR/mxe/usr/i686-w64-mingw32.static/share/cmake/mxe-conf.cmake \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DUID_PUBKEY_BYTES=`../int_to_bytes.py $n` \
  ..
make -j8
strip -s uid.exe
cd ..
