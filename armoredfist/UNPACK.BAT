@echo off
if exist install.bat del install.bat >nul
if exist copyr.exe del copyr.exe >nul
lha e /m1 /c fist.001
del fist.001 >nul
lha e /m1 /c fist.002
del fist.002 >nul
lha e /m1 /c fist.003
del fist.003 >nul
lha e /m1 /c fist.004
del fist.004 >nul
cd..
copy fistdata\*.exe .
copy fistdata\*.dvr .
copy fistdata\*.run .
copy fistdata\*.bat .
copy fistdata\fist.dat .
soundset
@echo.
@echo Armored Fist is Installed
@echo.
@echo Type "Fist" to play Armored Fist

