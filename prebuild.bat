@echo off

if "%VSINSTALLDIR%" == "" (
echo Visual Studio is not installed in this machine
goto eof
)

call "%VSINSTALLDIR%\VC\vcvarsall.bat" amd64

:eof