@echo off

if "%VS140COMNTOOLS%" == "" (
echo Visual Studio is not installed in this machine
goto eof
)

call "%VS140COMNTOOLS%\..\..\VC\vcvarsall.bat" amd64

:eof
