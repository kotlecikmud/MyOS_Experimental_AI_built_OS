@echo off
:: Build script for MyOS using WSL
:: This script assumes you have the necessary cross-compilers installed in your WSL distribution.

echo [WSL Build Runner] Launching build in WSL...

:: Run the make command inside WSL
wsl bash -c "make ARCH=x86_64"

if %ERRORLEVEL% NEQ 0 (
    echo [WSL Build Runner] Build failed.
    exit /b %ERRORLEVEL%
)

echo [WSL Build Runner] Build successful.

pause