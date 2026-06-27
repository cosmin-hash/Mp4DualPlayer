#!/bin/bash
echo "Building MP4 Player for Linux..."

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
echo "Building project..."
make -j$(nproc)

echo "Build complete!"
echo "Executable: build/Mpeg4Player"
echo ""
echo "To install system-wide (optional):"
echo "  sudo make install"
echo ""
echo "To run:"
echo "  ./Mpeg4Player"

