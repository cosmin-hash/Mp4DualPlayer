@echo off
echo Building MP4 Player version...

REM Create build directory if it doesn't exist
if not exist build mkdir build
cd build

REM Configure with CMake using MinGW
echo Configuring with CMake...
cmake .. -G "MinGW Makefiles"

REM Build the project
echo Building project...
cmake --build .

echo Build complete!
echo Executable should be in: build\Mpeg4Player.exe
pause
