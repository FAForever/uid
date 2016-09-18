#!/bin/bash

set -e

: ${n=13731707816857396218511477189051880183926672022487649441793167544537}

THIS_SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

[ -d jsoncpp-build ] || {
wget https://github.com/open-source-parsers/jsoncpp/archive/1.7.5.tar.gz -O jsoncpp.tar.gz
tar xfz jsoncpp.tar.gz
mkdir jsoncpp-build
cd jsoncpp-build
cmake \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF \
  -DBUILD_STATIC_LIBS=ON \
  -DBUILD_SHARED_LIBS=OFF \
  ../jsoncpp-1.7.5
make -j8
cd ..
}

[ -d cryptopp ] || {
wget https://github.com/weidai11/cryptopp/archive/CRYPTOPP_5_6_4.zip -O cryptopp.zip
unzip ./cryptopp.zip
mv cryptopp-CRYPTOPP_5_6_4 cryptopp
cd cryptopp
make -j8 -f GNUmakefile CXXFLAGS='-DCRYPTOPP_NO_BACKWARDS_COMPATIBILITY_562 -DNDEBUG -O3 -mtune=native -pipe' libcryptopp.a
cd ..
}

[ -d build ] || mkdir build
cd build
[ -f CMakeCache.txt ] || \
cmake \
  -DJSONCPP_LIBRARIES=$THIS_SCRIPT_DIR/jsoncpp-build/src/lib_json/libjsoncpp.a \
  -DJSONCPP_INCLUDE_DIRS=$THIS_SCRIPT_DIR/jsoncpp-1.7.5/include \
  -DCRYPTOPP_LIBRARIES=$THIS_SCRIPT_DIR/cryptopp/libcryptopp.a \
  -DCRYPTOPP_INCLUDE_DIRS=$THIS_SCRIPT_DIR \
  -DCMAKE_CXX_FLAGS="-DCRYPTOPP_NO_BACKWARDS_COMPATIBILITY_562" \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DUID_PUBKEY_BYTES=`../int_to_bytes.py $n` \
  ..
make -j8
strip -s uid
cd ..
