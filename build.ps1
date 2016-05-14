# assumptions:
# 1. You installed Python 2.7.11 to c:\python27
# 2. You installed nuitka
# 3. You have a working Visual studio installation (tested with 2010) and opened a Visual studio command prompt
# then you may run
# cmd /C "set n=13731707816857396218511477189051880183926672022487649441793167544537 && set e=65537 && powershell -ExecutionPolicy ByPass -File .\build.ps1"
# to build a uid.pyd file
(Get-Content .\uid\__init__.py) -replace '13731707816857396218511477189051880183926672022487649441793167544537', "$env:n" | Set-Content .\uid\__init__.py
(Get-Content .\uid\__init__.py) -replace '65537', "$env:e" | Set-Content .\uid\__init__.py
C:\Python27\Scripts\nuitka.bat --recurse-to=rsa --recurse-not-to=win32com.client --recurse-to=wmi --module uid --output-dir=build --remove-output
(Get-Content .\uid\__init__.py) -replace "$env:n", '13731707816857396218511477189051880183926672022487649441793167544537' | Set-Content .\uid\__init__.py
(Get-Content .\uid\__init__.py) -replace "$env:e", '65537' | Set-Content .\uid\__init__.py
