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
int_to_bytes.py 15016110316963555528094624349113466362595255853754721542742065372959
142,150,36,100,67,65,165,144,151,44,243,27,134,236,69,135,50,41,123,232,54,232,135,3,245,185,95,31
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

## Example key for the server

An example key for the server is provided in the file `faf-server.example.pem`.
The public key modulus `n` is `15016110316963555528094624349113466362595255853754721542742065372959`,
which translates to the pre-set bytes `142,150,36,100,67,65,165,144,151,44,243,27,134,236,69,135,50,41,123,232,54,232,135,3,245,185,95,31`.

The Base64-DER-encoded private key for the server, you need to provide is
`FAF_PRIVATE_KEY=MIGXAgEAAh0AjpYkZENBpZCXLPMbhuxFhzIpe+g26IcD9blfHwIDAQABAhwU8oHOKQNP63oKJHz6t5KY7jYsm3ZZubrLm4WBAg95Qr5XGWuT6fCZbnpFS88CDgEtBaEPHalBGWOQxPuxAg82LXTfc3MWLiKaWrr0dQ8CDSZLDgcPPeXDJhBXjrECDy6B4n8UrVVOIX/DC+Aakg==`.

## Key Generation

Generate private key: `openssl genrsa -out new_server_privkey.pem 2048`
Derive public key from private key: `openssl rsa -in new_server_privkey.pem -outform PEM -pubout -out new_server_pubkey.pem`

The private key for the server config has to be the base64-string in `new_server_privkey.pem` on one line without the armoring.

To fold it into one line: `tr -d '\n' < new_server_privkey.pem > new_server_privkey_nn.pem`
