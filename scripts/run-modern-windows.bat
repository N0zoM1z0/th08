@echo off
setlocal

set "TH08_RUNTIME=%~dp0"
set "TH08_EXE=%TH08_RUNTIME%th08-reconstructed.exe"

if not exist "%TH08_EXE%" goto missing_executable
if not exist "%TH08_RUNTIME%d3dx8d.dll" goto missing_d3dx
if not exist "%TH08_RUNTIME%th08.dat" goto missing_data
if not exist "%TH08_RUNTIME%thbgm.dat" goto missing_data

start "" /wait "%TH08_EXE%" --windowed --data-dir "%TH08_RUNTIME%."
set "TH08_EXIT=%ERRORLEVEL%"
if "%TH08_EXIT%"=="0" exit /b 0

echo.
echo TH08 reconstruction exited with code %TH08_EXIT%.
if exist "%TH08_RUNTIME%modern-crash.txt" (
    echo Crash details were written to:
    echo %TH08_RUNTIME%modern-crash.txt
)
pause
exit /b %TH08_EXIT%

:missing_executable
echo Missing reconstructed executable:
echo %TH08_EXE%
pause
exit /b 1

:missing_d3dx
echo Missing development DirectX runtime:
echo %TH08_RUNTIME%d3dx8d.dll
pause
exit /b 1

:missing_data
echo The playtest directory must contain th08.dat and thbgm.dat:
echo %TH08_RUNTIME%
pause
exit /b 1
