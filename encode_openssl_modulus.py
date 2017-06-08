#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
if len(sys.argv) < 2:
  sys.exit('You must provide the integer as first argument')

# treat the argument as openssl output `openssl rsa -noout -inform PEM -in /path/to/pubkey.pem -pubin -modulus`

if not sys.argv[1].startswith("Modulus="):
  sys.exit('Argument must start with "Modulus=". Please provide as first argument the output of >openssl rsa -noout -inform PEM -in /path/to/pubkey.pem -pubin -modulus')
  
hex_modulus = sys.argv[1].lstrip("Modulus=")
modulus_bytes = bytearray.fromhex(hex_modulus)
print(','.join(map(str, modulus_bytes)))
#print(len(modulus_bytes))
