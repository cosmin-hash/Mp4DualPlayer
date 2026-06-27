#!/bin/bash

echo "Building MP4 Player..."

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake ..

# Build the project
echo "Building project..."
cmake --build .

echo "Build complete!"
echo "Executable should be in: build/Mpeg4Player"



