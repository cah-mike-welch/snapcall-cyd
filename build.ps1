# SnapCall CYD Build Script
# Clears Python 2.7 path conflicts and runs PlatformIO build

$env:PYTHONPATH=''
$env:PYTHONHOME=''
C:\Python312\python.exe -m platformio run @args
