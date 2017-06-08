# FA Forever unique id implementation

This executable collects RSA encrypted user information.

It is used in the [FA Forever](http://www.faforever.com/) [client](https://github.com/FAForever/client) ([or this one](https://github.com/FAForever/downlords-faf-client)) to prevent [smurfing](http://www.urbandictionary.com/define.php?term=smurfing).

[![Travis](https://travis-ci.org/FAForever/uid.svg?branch=master)](https://travis-ci.org/FAForever/uid)


## Key generation instructions

Create a 2048 bit RSA key using openssl:
```
openssl genrsa -out faf_priv.pem 2048
```

Create the public key part which is stored encrypted in this repo:
```
openssl rsa -in faf_priv.pem -pubout -out faf_pub.pem
```

Now generate the UID_PUBKEY_BYTES string variable you need to pass to CMake with
```
./encode_openssl_modulus.py `openssl rsa -noout -inform PEM -in faf_pub.pem -pubin -modulus`
```
which should result in
```
200,172,159,159,117,211,197,121,7,80,96,139,82,253,240,67,219,77,244,65,25,119,3,147,242,142,113,95,120,226,30,104,158,211,48,73,96,59,85,198,183,199,146,127,140,87,183,110,75,173,39,218,35,146,181,21,115,29,238,23,204,109,15,252,99,204,103,130,138,78,12,7,65,20,247,29,195,136,20,53,200,221,58,114,11,170,65,151,100,61,139,170,244,158,7,192,99,91,142,217,139,253,106,198,180,112,173,49,106,90,121,163,2,24,206,176,198,187,35,37,111,218,197,202,247,139,30,126,152,38,34,73,25,199,10,194,12,196,144,195,98,48,149,14,219,39,182,154,73,246,96,81,152,95,163,251,8,117,35,226,61,16,164,190,128,239,187,122,78,102,209,233,11,126,80,71,187,78,239,28,48,175,91,51,100,83,165,203,222,119,117,138,82,131,199,90,134,250,62,51,231,180,158,11,109,138,75,37,221,145,184,14,177,203,192,191,48,25,159,137,191,252,5,174,209,207,247,198,32,56,152,65,134,251,180,147,36,250,95,50,253,103,240,3,100,211,86,117,5,63,205,61,176,76,48,209
```
for the provided example keyfile `faf_priv_example.pem`.

The private key must be supplied to the server as base64 encoded DER encoded environment variable FAF_PRIVATE_KEY you can generate with
```
openssl rsa -inform PEM -in ./faf_priv.pem -outform DER | openssl enc -base64
```

## Build instructions
You need:
* [CMake](https://cmake.org/)
* [Crypto++](https://www.cryptopp.com/)
* [JsonCpp](https://open-source-parsers.github.io/jsoncpp-docs/doxygen/index.html)
* a recent C++ compiler
* [UID_PUBKEY_BYTES bytes string](## Key generation instructions)

Example build commandline script:
```bash
mkdir build
cd build
cmake \
  -DJSONCPP_LIBRARIES=/path/to/libjsoncpp.a \
  -DJSONCPP_INCLUDE_DIRS=/path/to/jsoncpp/include \
  -DCRYPTOPP_LIBRARIES=/path/to/libcryptopp.a \
  -DCRYPTOPP_INCLUDE_DIRS=/path/to/croptopp/include \
  -DUID_PUBKEY_BYTES=200,172,... \
  ..
make
```
