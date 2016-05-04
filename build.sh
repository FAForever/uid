#!/bin/sh

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

sed -i "s,n = 0,n = $n,g" './uid/rsakeys.py'
sed -i "s,e = 0,e = $e,g" './uid/rsakeys.py'

nuitka --python-version='2.7' --recurse-to=uid.rsakeys --module uid
