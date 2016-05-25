# FA Forever unique id implementation

This Python module collects RSA encrypted user information.

It is used as compiled Python extension in the [FA Forever client](https://github.com/FAForever/client) to prevent [smurfing](http://www.urbandictionary.com/define.php?term=smurfing).

[![Appveyor](https://ci.appveyor.com/api/projects/status/kpphf58np8vd72rw?svg=true)](https://ci.appveyor.com/project/muellni/uid)
[![Travis](https://travis-ci.org/FAForever/uid.svg?branch=master)](https://travis-ci.org/muellni/uid)

## Build instructions
You need:
* [nuitka](https://nuitka.net/)
* [wmi](https://pypi.python.org/pypi/WMI/) (Windows only)
* [PyCrypto](https://pypi.python.org/pypi/pycrypto)
* [rsa](https://pypi.python.org/pypi/rsa)

You need the _real_ Public Key components `n` and `e`.
### Windows
Run `cmd /C "set n=13731707816857396218511477189051880183926672022487649441793167544537 && set e=65537 && powershell -ExecutionPolicy ByPass -File .\build.ps1"` in the root directory to create `.\build\uid.pyd`.
### Linux
Run `e='65537' n='13731707816857396218511477189051880183926672022487649441793167544537' ./build.sh` in the root directory to create `./build/uid.so`.
