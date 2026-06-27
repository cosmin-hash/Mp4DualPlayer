# MP4 Player - Linux Build Instructions

## Prerequisites

### Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential cmake
sudo apt install qt6-base-dev qt6-multimedia-dev qt6-tools-dev
sudo apt install libavcodec-dev libavformat-dev libavutil-dev
```

### Fedora/RHEL:
```bash
sudo dnf install gcc-c++ cmake
sudo dnf install qt6-qtbase-devel qt6-qtmultimedia-devel
sudo dnf install ffmpeg-devel
```

### Arch Linux:
```bash
sudo pacman -S base-devel cmake
sudo pacman -S qt6-base qt6-multimedia
sudo pacman -S ffmpeg
```

## Building

1. **Make the build script executable:**
   ```bash
   chmod +x build_linux.sh
   ```

2. **Run the build script:**
   ```bash
   ./build_linux.sh
   ```

3. **Or build manually:**
   ```bash
   mkdir build
   cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   make -j$(nproc)
   ```

## Running

- **From build directory:**
  ```bash
  ./Mpeg4Player
  ```

- **Install system-wide (optional):**
  ```bash
  sudo make install
  ```
  Then run from anywhere:
  ```bash
  Mpeg4Player
  ```

## Features

- ✅ Cross-platform Qt6 multimedia support
- ✅ Advanced C++20 features (concepts, coroutines)
- ✅ Dual display video playback
- ✅ Software rendering (no OpenGL dependencies)
- ✅ Clean shutdown handling
- ✅ Linux desktop integration

## Troubleshooting

- **Qt6 not found:** Install Qt6 development packages
- **FFmpeg issues:** Install FFmpeg development libraries
- **Permission denied:** Make sure build script is executable (`chmod +x build_linux.sh`)
- **Missing libraries:** Check that all Qt6 multimedia plugins are installed

## Dependencies

- Qt6 (Base, Multimedia, Widgets)
- FFmpeg libraries (for video codecs)
- CMake 3.16+
- GCC with C++20 support

