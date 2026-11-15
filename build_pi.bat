@echo off
echo [WSL Build Runner] Building for Raspberry Pi Zero W (armv6)...
wsl bash -c "make ARCH=armv6"
if %ERRORLEVEL% NEQ 0 (
    echo [WSL Build Runner] Build failed.
    exit /b %ERRORLEVEL%
)
echo [WSL Build Runner] Build successful.

pause