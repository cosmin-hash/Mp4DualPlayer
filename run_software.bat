@echo off
echo Starting MPEG4 Player
echo =====================

cd /d "%~dp0build"

if not exist "Mpeg4Player.exe" (
    echo ERROR: Mpeg4Player.exe not found!
    echo Please run build_release.bat first.
    pause
    exit /b 1
)

echo Launching MPEG4 Player...
.\Mpeg4Player.exe

pause
