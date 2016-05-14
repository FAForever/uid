#!/bin/sh

use_real_keys=true

if [ "$use_real_keys" = true ] ; then
  if ! env | grep -q ^n=
  then
    echo "You need to set the environment variable 'n' to the public key parameter n"
    exit 1
  fi

  if ! env | grep -q ^e=
  then
    echo "You need to set the environment variable 'e' to the public key parameter e"
    exit 1
  fi

  rsa_test_n=13731707816857396218511477189051880183926672022487649441793167544537
  rsa_test_e=65537
  rsa_test_d=13257759923579836515652436320509365545753327507215179875192524262973
  rsa_test_p=559894335379760802227172596053317511
  rsa_test_q=24525534460968531815139548638367

  sed -i "s,$rsa_test_n,$n,g" './uid/__init__.py'
  sed -i "s,$rsa_test_e,$e,g" './uid/__init__.py'
fi

nuitka --python-version='2.7' --recurse-to=rsa --module uid --output-dir=build --remove-output

if [ "$use_real_keys" = true ] ; then
  sed -i "s,$n,$rsa_test_n,g" './uid/__init__.py'
  sed -i "s,$e,$rsa_test_e,g" './uid/__init__.py'
fi
