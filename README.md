# FA Forever unique id implementation

This executable collects RSA encrypted user information.

It is used in the [FA Forever](http://www.faforever.com/) [client](https://github.com/FAForever/client) ([or this one](https://github.com/FAForever/downlords-faf-client)) to prevent [smurfing](http://www.urbandictionary.com/define.php?term=smurfing).

[![Travis](https://travis-ci.org/FAForever/uid.svg?branch=master)](https://travis-ci.org/FAForever/uid)

## Build instructions
You need:
* [CMake](https://cmake.org/)
* [Crypto++](https://www.cryptopp.com/)
* [JsonCpp](https://open-source-parsers.github.io/jsoncpp-docs/doxygen/index.html)
* a recent C++ compiler

You need the _real_ Public Key component `n` (aka modulus). It is fed to the compiler via a byte list as CMake variable `UID_PUBKEY_BYTES`.
This list can be generated from an integer string using the `int_to_bytes.py` script:
```
int_to_bytes.py 13731707816857396218511477189051880183926672022487649441793167544537
130,99,238,192,232,47,187,99,222,116,140,101,233,231,57,188,204,204,187,241,173,147,88,60,217,7,80,217
```

Example build commandline script:
```bash
n=13731707816857396218511477189051880183926672022487649441793167544537
mkdir build
cd build
cmake \
  -DJSONCPP_LIBRARIES=/path/to/libjsoncpp.a \
  -DJSONCPP_INCLUDE_DIRS=/path/to/jsoncpp/include \
  -DCRYPTOPP_LIBRARIES=/path/to/libcryptopp.a \
  -DCRYPTOPP_INCLUDE_DIRS=/path/to/croptopp/include \
  -DUID_PUBKEY_BYTES=`../int_to_bytes.py $n` \
  ..
make
```
