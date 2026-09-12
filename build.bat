@echo off
REM SnapCall CYD Build Script (Windows CMD)
REM Clears Python 2.7 path conflicts and runs PlatformIO build

set PYTHONPATH=
set PYTHONHOME=
C:\Python312\python.exe -m platformio run %*
