#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys

def exit():
  sys.exit('Argument must start with "Modulus=". Please provide as first argument the output of >openssl rsa -noout -inform PEM -in faf_pub.pem -pubin -modulus')

if len(sys.argv) < 2:
  exit()

if not sys.argv[1].startswith("Modulus="):
  exit()

  # treat the argument as openssl output `openssl rsa -noout -inform PEM -in /path/to/pubkey.pem -pubin -modulus`
hex_modulus = sys.argv[1].lstrip("Modulus=")
modulus_bytes = bytearray.fromhex(hex_modulus)
print(','.join(map(str, modulus_bytes)))
