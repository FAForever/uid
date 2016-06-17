#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
if len(sys.argv) < 2:
  sys.exit('You must provide the integer as first argument')
print(','.join(map(str, int(sys.argv[1]).to_bytes(28, byteorder='big'))))
