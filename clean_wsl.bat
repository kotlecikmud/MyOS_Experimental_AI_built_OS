@echo off
:: Clean script for MyOS using WSL

echo [WSL Clean Runner] Cleaning build artifacts in WSL...

wsl bash -c "make clean"

if %ERRORLEVEL% NEQ 0 (
    echo [WSL Clean Runner] Clean failed.
    exit /b %ERRORLEVEL%
)

echo [WSL Clean Runner] Clean successful.

pause