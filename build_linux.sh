#!/bin/bash

#set -e

: ${n=13731707816857396218511477189051880183926672022487649441793167544537}

THIS_SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

wget https://github.com/open-source-parsers/jsoncpp/archive/1.7.2.tar.gz -O jsoncpp.tar.gz
tar xfz jsoncpp.tar.gz
mkdir jsoncpp-build
cd jsoncpp-build
cmake \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF \
  -DBUILD_STATIC_LIBS=ON \
  -DBUILD_SHARED_LIBS=OFF \
  ../jsoncpp-1.7.2
make -j8
cd ..

wget https://www.cryptopp.com/cryptopp563.zip
mkdir cryptopp
cd cryptopp
unzip ../cryptopp563.zip
make -j8 -f GNUmakefile CXXFLAGS='-DNDEBUG -O3 -mtune=native -pipe' libcryptopp.a
cd ..

mkdir build
cd build
cmake \
  -DJSONCPP_LIBRARIES=$THIS_SCRIPT_DIR/jsoncpp-build/src/lib_json/libjsoncpp.a \
  -DJSONCPP_INCLUDE_DIRS=$THIS_SCRIPT_DIR/jsoncpp-1.7.2/include \
  -DCRYPTOPP_LIBRARIES=$THIS_SCRIPT_DIR/cryptopp/libcryptopp.a \
  -DCRYPTOPP_INCLUDE_DIRS=$THIS_SCRIPT_DIR \
  -DCMAKE_BUILD_TYPE=MinSizeRel \
  -DUID_PUBKEY_BYTES=`./int_to_bytes.py $n` \
  ..
make -j8
strip -s uid
cd ..
