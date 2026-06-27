 @echo off
echo Building MPEG4 Player - Release Version (Optimized)
echo ==================================================

REM Clean previous build
echo Cleaning previous build...
if exist build rmdir /s /q build
mkdir build

REM Configure with Release build type
echo Configuring CMake for Release build...
cd build
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..

if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake configuration failed!
    pause
    exit /b 1
)

REM Build with maximum optimization
echo Building with -O3 optimization...
mingw32-make -j4

if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo.
echo ================================================
echo BUILD SUCCESSFUL!
echo ================================================
echo.
echo Optimizations applied:
echo - O3 maximum optimization
echo - Native CPU architecture tuning
echo - Link-time optimization (LTO)
echo - Fast math operations
echo - Loop unrolling
echo - Function inlining
echo - Debug output minimized
echo.
echo Executable: build\Mpeg4Player.exe
echo.
echo Usage:
echo   .\Mpeg4Player.exe              - Auto-detect rendering
echo   .\Mpeg4Player.exe --opengl     - Force OpenGL rendering
echo   .\Mpeg4Player.exe --software   - Force software rendering
echo   .\Mpeg4Player.exe --help       - Show help
echo.
pause

