@echo off
setlocal

set "PATCH_SCRIPT=%~dp0run-preserve-lives-test.ps1"
if not exist "%PATCH_SCRIPT%" (
    echo Missing runtime patch helper: "%PATCH_SCRIPT%"
    pause
    exit /b 1
)

powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%PATCH_SCRIPT%"
if errorlevel 1 (
    echo.
    echo The preserve-lives test launch failed. The normal executable was not modified.
    pause
    exit /b 1
)

endlocal
