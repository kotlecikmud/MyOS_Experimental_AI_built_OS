@echo off
cls

echo ==============================
echo   Starting update process...
echo ==============================

echo.
echo Pulling latest code from Git...
git pull
if errorlevel 1 (
echo ERROR: git pull failed!
)

echo.
echo ==============================
echo   Update completed successfully!
echo ==============================

pause
