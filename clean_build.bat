@echo off
echo Cleaning build directory...

REM Kill any remaining Mpeg4Player processes
taskkill /F /IM Mpeg4Player.exe 2>nul

REM Wait a moment for processes to fully terminate
timeout /t 2 /nobreak >nul

REM Remove build directory
if exist build (
    echo Removing build directory...
    rmdir /s /q build
    echo Build directory removed.
) else (
    echo Build directory does not exist.
)

echo Cleanup complete!
pause

