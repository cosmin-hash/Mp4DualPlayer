# C++20/23 Modernization Roadmap for MPEG4 Player

## 🎯 Overview
This document outlines advanced C++20/23 features that can be implemented in the MPEG4 Player project to improve code quality, performance, and maintainability.

> **Current baseline:** The project compiles as **C++20** (`CMAKE_CXX_STANDARD 20` in `CMakeLists.txt`). Items below are tagged **[C++20]** (buildable on the current GCC 13.1.0 toolchain) or **[C++23]** (require raising the standard, and in some cases a newer compiler — see each item).

## 📋 Implementation Checklist

### 1. Enhanced Concepts & Constraints **[C++20]**
- [ ] **Priority: HIGH** - Convert existing SFINAE/type-traits to concepts
- [ ] Replace the `is_qt_widget` / `is_qt_object` / `enable_if_*` traits in `advanced_cpp_features.h` with `concept` definitions
- [ ] Create `QtObject` concept for Qt object validation
- [ ] Create a `MediaController` concept covering the `MediaWidget` / `VideoWidgetQt` playback interface
- [ ] Convert the existing `safeConnect` (currently SFINAE/`enable_if`-based, used ~25 times) to a concept-constrained template
- [ ] Add compile-time validation for signal/slot connections

**Files to modify:**
- `src/advanced_cpp_features.h`
- `src/videowidget_qt.cpp`
- `src/mediawidget.cpp`

**Benefits:**
- Better error messages when connecting wrong object types
- Compile-time validation of media player interfaces
- More readable and maintainable code

---

### 2. `std::expected<T, E>` - Modern Error Handling **[C++23]**
> Requires raising the standard to C++23. Supported by the current GCC 13.1.0 libstdc++.
- [ ] **Priority: HIGH** - Replace exception-based error handling
- [ ] Define `MediaError` enum for error types
- [ ] Create `createMediaPlayer()` function with expected return type
- [ ] Implement `loadVideo()` with expected error handling
- [ ] Update error handling in `VideoWidgetQt::onErrorOccurred()`
- [ ] Add error handling for file operations

**Files to modify:**
- `src/videowidget_qt.cpp`
- `src/mediawidget.cpp`
- `src/mainwindow.cpp`

**Benefits:**
- No exception handling overhead
- Explicit error handling that's hard to ignore
- Better performance than exceptions
- Type-safe error propagation

---

### 3. Enhanced Ranges & Views **[C++20]**
- [ ] **Priority: MEDIUM** - Optimize frame processing
- [ ] Implement `processFrames()` with ranges
- [ ] Create frame processing pipeline with views
- [ ] Add batch processing for video frames
- [ ] Optimize UI element management with ranges

**Files to modify:**
- `src/videowidget_qt.cpp`
- `src/mediawidget.cpp`

**Benefits:**
- More efficient frame processing
- Composable transformations
- Lazy evaluation (only processes what's needed)
- Better memory usage

---

### 4. `consteval` Functions - Compile-Time Computation **[C++20]**
- [ ] **Priority: MEDIUM** - Add compile-time validation
- [ ] Create `isValidVideoFormat()` consteval function
- [ ] Implement `getOptimalBufferSize()` for buffer calculation
- [ ] Add compile-time media player configuration
- [ ] Create template-based configuration system

**Files to modify:**
- `src/advanced_cpp_features.h`
- `src/videowidget_qt.cpp`

**Benefits:**
- Zero runtime overhead for configuration
- Compile-time validation of video formats
- Better optimization opportunities
- Catch errors at compile-time

---

### 5. Enhanced `std::format` with Ranges **[C++23 — needs GCC 14+]**
> Basic `std::format` works on GCC 13, but **range-formatting** (formatting containers directly) landed in **GCC 14**. This item requires a compiler upgrade.
- [ ] **Priority: LOW** - Improve logging and debugging
- [ ] Replace `qDebug()` statements with `std::format`
- [ ] Implement `logMediaStatus()` with format
- [ ] Add `logPlaybackInfo()` for playback information
- [ ] Create `logSupportedFormats()` for format logging

**Files to modify:**
- `src/videowidget_qt.cpp`
- `src/mediawidget.cpp`
- `src/mainwindow.cpp`

**Benefits:**
- More efficient string formatting
- Better performance than string concatenation
- Type-safe formatting
- Cleaner, more readable code

---

### 6. `std::ranges::to` - Container Conversion **[C++23 — needs GCC 14+]**
> `std::ranges::to` (P1206) is **not available in GCC 13.1.0**; it was added in **GCC 14**. This item requires a compiler upgrade.
- [ ] **Priority: LOW** - Streamline data transformations
- [ ] Implement `setupQualityOptions()` with ranges
- [ ] Add `getVideoInfo()` for frame information
- [ ] Optimize UI element creation with container conversion
- [ ] Streamline data processing pipelines

**Files to modify:**
- `src/mediawidget.cpp`
- `src/videowidget_qt.cpp`

**Benefits:**
- More efficient data transformations
- Cleaner code for UI updates
- Better memory management
- Functional programming style

---

## 🚀 Recommended Implementation Order

### Phase 1: Foundation (High Priority)
1. **Enhanced Concepts & Constraints**
   - Start with `QtObject` and `MediaPlayer` concepts
   - Update `safeConnect` function
   - Add compile-time validation

2. **`std::expected<T, E>` Error Handling**
   - Define error types
   - Implement safe media operations
   - Replace exception-based error handling

### Phase 2: Performance (Medium Priority)
3. **`consteval` Functions**
   - Add compile-time validation
   - Implement configuration system
   - Optimize buffer calculations

4. **Enhanced Ranges & Views**
   - Optimize frame processing
   - Implement processing pipelines
   - Add batch operations

### Phase 3: Polish (Low Priority)
5. **Enhanced `std::format`**
   - Improve logging system
   - Replace string concatenation
   - Add type-safe formatting

6. **`std::ranges::to`**
   - Streamline data transformations
   - Optimize UI updates
   - Improve container operations

---

## 📁 File Structure Changes

### New Files to Create:
- `src/modern_cpp_features.h` - C++20/23 feature implementations
- `src/error_handling.h` - Error types and `std::expected` utilities
- `src/concepts.h` - Custom concepts definitions

### Files to Modify:
- `src/advanced_cpp_features.h` - Update with C++23 features
- `src/videowidget_qt.cpp` - Implement new error handling and ranges
- `src/mediawidget.cpp` - Add concepts and format improvements
- `src/mainwindow.cpp` - Update error handling
- `CMakeLists.txt` - Ensure C++23 standard is set

---

## 🔧 Configuration Requirements

### CMakeLists.txt Updates:
The project currently sets `CMAKE_CXX_STANDARD 20`. Items 1, 3, and 4 work as-is on C++20.
Items 2, 5, and 6 require raising the standard to C++23:
```cmake
set(CMAKE_CXX_STANDARD 23)        # only needed for std::expected / std::format-ranges / std::ranges::to
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Note: -fconcepts is a no-op under C++20/23 on modern GCC (concepts are standard).
# -std=c++23 also enables coroutines; -fcoroutines is not required on GCC 13+.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23")
endif()
```

### Required Headers:
```cpp
#include <concepts>
#include <expected>
#include <ranges>
#include <format>
#include <span>
```

---

## 📊 Success Metrics

### Code Quality:
- [ ] Reduced compilation errors due to type mismatches
- [ ] Improved error handling coverage
- [ ] Better compile-time validation

### Performance:
- [ ] Reduced runtime overhead in error handling
- [ ] More efficient frame processing
- [ ] Better memory usage patterns

### Maintainability:
- [ ] More readable and self-documenting code
- [ ] Easier debugging with better error messages
- [ ] Cleaner separation of concerns

---

## 🎯 Next Steps

1. **Review this roadmap** and prioritize features based on project needs
2. **Choose starting point** from Phase 1 recommendations
3. **Implement incrementally** - one feature at a time
4. **Test thoroughly** after each implementation
5. **Update documentation** as features are added

---

## 📝 Notes

- Toolchain support for the listed features:
  - MinGW GCC 13.1.0 — supports concepts, ranges, `consteval`, `std::format` (basic), and `std::expected`. **Does NOT support `std::ranges::to` or `std::format` range-formatting** (Items 5 & 6) — those need **GCC 14+**.
  - CMake 4.1.1 ✅
  - Qt 6.9.2 ✅

- Implementation should be done incrementally to avoid breaking existing functionality
- Each feature can be implemented independently
- Consider creating feature branches for each major implementation

---

*Project: MPEG4 Player (Qt Widgets) — C++20/23 modernization roadmap*
