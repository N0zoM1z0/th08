@echo off
setlocal

set "TH08_RUNTIME=%~dp0"
set "TH08_EXE=%TH08_RUNTIME%th08-reconstructed.exe"

if not exist "%TH08_EXE%" goto missing_executable
if not exist "%TH08_RUNTIME%th08.dat" goto missing_data
if not exist "%TH08_RUNTIME%thbgm.dat" goto missing_data

if not exist "%TH08_RUNTIME%th08.cfg" (
    echo Warning: th08.cfg is missing. Configure windowed mode before endurance testing.
    echo.
)

start "" /wait "%TH08_EXE%"
set "TH08_EXIT=%ERRORLEVEL%"
if "%TH08_EXIT%"=="0" exit /b 0

echo.
echo TH08 reconstruction exited with code %TH08_EXIT%.
echo Record the executable SHA-256 and the last tested transition.
pause
exit /b %TH08_EXIT%

:missing_executable
echo Missing native VC7 reconstruction:
echo %TH08_EXE%
pause
exit /b 1

:missing_data
echo The isolated playtest directory must contain th08.dat and thbgm.dat:
echo %TH08_RUNTIME%
pause
exit /b 1
