# MP4 Player - Qt Widgets Version

> A dual-display MP4 media player built with Qt 6 Widgets and modern C++20.

![CI](https://github.com/cosmin-hash/Mp4DualPlayer/actions/workflows/ci.yml/badge.svg)
![Qt](https://img.shields.io/badge/Qt-6-41CD52?logo=qt&logoColor=white)
![C++](https://img.shields.io/badge/C%2B%2B-20-00599C?logo=cplusplus&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.16%2B-064F8C?logo=cmake&logoColor=white)
![Platforms](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey)
![License](https://img.shields.io/badge/license-MIT-green)

## Demo

_Dual-display playback with live FPS and resolution overlays._

https://github.com/user-attachments/assets/fbdabfc8-e881-4ef0-a88b-1bf46633d879

A modern Qt Widgets-based dual display MP4 media player with advanced C++20 features.

## Features

- ✅ **Dual Display Support** - Two independent video players side by side
- ✅ **Native Qt Widgets UI** - Responsive Qt Widgets interface
- ✅ **Advanced C++20** - Concepts, template metaprogramming, and perfect forwarding
- ✅ **Cross-Platform** - Windows, Linux, and macOS support
- ✅ **Performance Optimized** - Frame rate limiting and quality settings
- ✅ **Clean Architecture** - Separation of media logic and widget UI

## Architecture

### C++ Backend
- **MediaWidget** - Core media playback logic and player UI (`QWidget`)
- **VideoWidgetQt** - Custom `QWidget` for video rendering
- **VideoProcessingThread** - `QThread` for off-thread video processing
- **Advanced C++20 Features** - Template metaprogramming, concepts, perfect forwarding

### Qt Widgets Frontend
- **MainWindow** - Main application window (`QMainWindow`) hosting the dual players
- **Responsive Design** - Adaptive layouts and styling
- **Real-time Updates** - Live FPS, resolution, and position tracking

### Data Flow

Each player moves a decoded frame from Qt Multimedia, through a single-slot
drop queue, onto a worker thread for scaling, and back to the GUI thread for
painting:

```
 QMediaPlayer  ──▶  QVideoSink  ──▶  frame-drop queue  ──▶  VideoProcessingThread  ──▶   GUI thread
 (decode via        (per-frame      (one pending frame;     (scale + transform off       (QLabel shows
  Qt Multimedia)     callback)       newest frame wins)      the GUI thread)               the QPixmap)
       │                  │                  │                       │                          ▲
       └ demux/decode     └ emits            └ guarded by            └ QVideoFrame → QImage      └ queued
         (FFmpeg etc.)      videoFrameChanged   QMutex/QWaitCondition   → scaled QPixmap            frameProcessed signal
```

## Design notes

**Producer / consumer with intentional frame-dropping.** The GUI thread is the
producer: `QVideoSink::videoFrameChanged` hands it each decoded frame, which it
forwards to `VideoProcessingThread` (the consumer). The queue holds exactly one
pending frame — if the worker is still busy scaling the previous frame when a new
one arrives, `processFrame()` drops the new frame rather than buffering it. This
bounds memory and latency under load, deliberately favouring smooth, low-latency
playback over delivering every single frame. A frame-rate limiter (15 / 30 / 60
FPS, by quality setting) further thins the stream before it reaches the queue.

**Synchronization.** The single-slot queue is guarded by a `QMutex`, with a
`QWaitCondition` coordinating producer and consumer. The worker blocks on
`m_condition.wait(&m_mutex)` and is woken only when there is real work — a new
frame (`processFrame()` calls `wakeOne()`) or a shutdown request
(`stop()` calls `wakeAll()`). There is no polling timeout, so an idle worker
consumes no CPU.

**Cooperative shutdown.** Teardown never uses `QThread::terminate()`. `stop()`
sets a `m_stopRequested` flag and wakes the condition; the thread's destructor
then calls `wait()` to join it. The worker re-checks `m_stopRequested` at each
stage of `run()`, so it always exits cleanly between frames and releases its
resources deterministically — no thread is killed mid-operation.

**Decoding vs. post-processing.** All demuxing and decoding is handled by Qt
Multimedia (`QMediaPlayer` plus the platform backend, e.g. FFmpeg). The worker
thread only post-processes already-decoded frames: `QVideoFrame` → `QImage`,
aspect-correct scaling to the label size, and the quality-dependent
transformation mode. The finished `QPixmap` is sent back to the GUI thread via a
queued `frameProcessed` signal, because Qt widgets may only be painted on the
GUI thread.

## Building from Source

### Prerequisites (all platforms)
- **Qt 6** with the `Core`, `Gui`, `Widgets`, and `Multimedia` modules
- **CMake** 3.16 or newer
- A **C++20** compiler (GCC 13+, Clang 16+, or MSVC 2022)

The build locates Qt via `CMAKE_PREFIX_PATH`. Point it at your Qt installation
in any of these ways:
- Select a CMake **kit** in your IDE (VS Code / Qt Creator), **or**
- pass `-DCMAKE_PREFIX_PATH=<Qt>/<version>/<compiler>/lib/cmake` at configure time, **or**
- set the `QT_DIR` environment variable to that path.

On Windows, if Qt is installed at the default location
(`C:/Qt/6.9.2/mingw_64` with the bundled MinGW), no extra configuration is
needed — the included fallbacks are used automatically.

### Windows (MinGW)
```bat
:: Default Qt install location — just run the script
.\build.bat

:: ...or configure explicitly against a different Qt
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/Qt/6.9.2/mingw_64/lib/cmake"
cmake --build build
```

### Linux
```bash
# Install dependencies (Debian/Ubuntu)
sudo apt install build-essential cmake qt6-base-dev qt6-multimedia-dev

# Build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### macOS
```bash
brew install qt cmake
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build -j$(sysctl -n hw.ncpu)
```

> **Tip:** For a maximally optimized local build tuned to your own CPU, add
> `-DENABLE_NATIVE_OPTIMIZATION=ON`. The resulting binary is **not portable**
> to other machines, so leave it off when building for distribution.

## VS Code

This repo includes a `.vscode/` config for the CMake Tools and C/C++
extensions. On first build, CMake Tools prompts **"Select a Kit"** — choose your
GCC/MinGW (or other C++20) toolchain. Machine-specific overrides can be placed in
`.vscode/settings.local.json` (gitignored).

## Usage

1. **Launch the application**
2. **Load videos** - Click "Open" on either display to load MP4 files
3. **Control playback** - Use Play/Pause/Stop buttons
4. **Adjust quality** - Select Low/Medium/High quality settings
5. **Monitor performance** - View real-time FPS and resolution info

## Technical Details

### Qt Widgets Integration
- UI built from `QWidget`/`QMainWindow` subclasses
- Signal/slot connections between widgets and media logic
- Playback driven by Qt Multimedia (`QMediaPlayer`)

### Performance Features
- Frame rate limiting (15/30/60 FPS)
- Quality-based rendering settings
- Efficient video frame processing
- Memory-optimized video handling

### Cross-Platform Support
- Windows: MinGW with Qt6
- Linux: System Qt6 packages
- macOS: System Qt6 installation

## Dependencies

- Qt6 (Core, Gui, Widgets, Multimedia)
- CMake 3.16+
- C++20 compatible compiler
- FFmpeg libraries (via Qt Multimedia)

## File Structure

```
Mp4DualPlayer/
├── src/
│   ├── main.cpp              # Application entry point
│   ├── mainwindow.h/cpp      # Main application window
│   ├── mediawidget.h/cpp     # Core media player logic and UI
│   └── videowidget_qt.h/cpp  # Video rendering widget
├── CMakeLists.txt            # Build configuration
└── build.bat                 # Windows build script
```
