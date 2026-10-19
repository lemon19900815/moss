@echo off
cd /d "%~dp0"
helloworld.exe %*
echo EXITCODE=%errorlevel%
