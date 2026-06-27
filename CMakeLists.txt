cmake_minimum_required(VERSION 3.16)

# ---------------------------------------------------------------------------
# Toolchain / Qt6 discovery
#
# Qt6 is located via CMAKE_PREFIX_PATH. Provide it in whichever way suits you
# (highest precedence first):
#   1. Select a CMake kit in your IDE (provides the C/C++ compiler), and/or
#   2. Configure with: -DCMAKE_PREFIX_PATH=<Qt>/<version>/<compiler>/lib/cmake
#   3. Set the QT_DIR environment variable to that same path
#
# If none of the above are given, the Windows fallbacks below assume a standard
# Qt online-installer layout. They only apply when nothing else has been set,
# so they never override a kit or an explicit -DCMAKE_PREFIX_PATH=...
# ---------------------------------------------------------------------------
if(WIN32)
    # Fallback Qt6 location (standard Qt installer default). Override with
    # -DCMAKE_PREFIX_PATH=... if your Qt is a different version or location.
    if(NOT CMAKE_PREFIX_PATH AND NOT DEFINED ENV{QT_DIR})
        set(CMAKE_PREFIX_PATH "C:/Qt/6.9.2/mingw_64/lib/cmake")
    endif()

    # Fallback MinGW toolchain — used only if a kit/toolchain has not already
    # selected a compiler, and only if this default install actually exists.
    if(NOT CMAKE_CXX_COMPILER AND EXISTS "C:/Qt/Tools/mingw1310_64/bin/g++.exe")
        set(CMAKE_GENERATOR "MinGW Makefiles" CACHE INTERNAL "" FORCE)
        set(CMAKE_CXX_COMPILER "C:/Qt/Tools/mingw1310_64/bin/g++.exe" CACHE INTERNAL "" FORCE)
        set(CMAKE_C_COMPILER "C:/Qt/Tools/mingw1310_64/bin/gcc.exe" CACHE INTERNAL "" FORCE)
        set(CMAKE_MAKE_PROGRAM "C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe" CACHE INTERNAL "" FORCE)
    endif()
elseif(UNIX AND NOT APPLE)
    # Linux-specific: Let CMake find system Qt6
    message(STATUS "Linux build detected - using system Qt6")
elseif(APPLE)
    # macOS-specific: Let CMake find system Qt6
    message(STATUS "macOS build detected - using system Qt6")
endif()

project(Mpeg4Player VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # Use -std=c++20 instead of -std=gnu++20

# Additional C++20 compiler flags for GCC (Windows MinGW and Linux)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
    # Enable key C++20 features
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts")
    message(STATUS "GCC compiler detected - C++20 features enabled")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines-ts")
    message(STATUS "Clang compiler detected - C++20 features enabled")
endif()

# Release build optimization
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# Native CPU tuning produces faster but NON-PORTABLE binaries (will not run on a
# different/older CPU). Off by default so the project is safe to build and share;
# enable with -DENABLE_NATIVE_OPTIMIZATION=ON for a local performance build.
option(ENABLE_NATIVE_OPTIMIZATION "Tune the build for this machine's CPU (-march=native, -ffast-math). Non-portable." OFF)

# Optimize for performance
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -funroll-loops -finline-functions")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DQT_NO_DEBUG_OUTPUT")
    if(ENABLE_NATIVE_OPTIMIZATION)
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -mtune=native -ffast-math")
        message(STATUS "Release build: native CPU tuning ENABLED (binary is not portable to other CPUs)")
    endif()
    message(STATUS "Release build: Maximum optimization enabled, debug output disabled")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
    message(STATUS "Debug build: Debug symbols enabled")
endif()

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Multimedia)

# Enable Qt6 MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Disable automatic generation/deployment of translations and QML language server INI
# This prevents the build from creating .ts/.qm translation outputs and the
# qml language server INI used by some Qt tooling.
set(QT_QML_GENERATE_QMLLS_INI OFF CACHE BOOL "Disable QML language server INI generation" FORCE)
# Disable QtDeploy translations directory so deploy helpers don't install translations
set(QT_DEPLOY_TRANSLATIONS_DIR "" CACHE STRING "Disable deploy translations dir" FORCE)
## Try to disable Qt translation feature in CMake cache to avoid translation steps
## Note: Do not override QT_FEATURE_translation here; Qt's package controls this.

# Source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/videowidget_qt.cpp
    src/mediawidget.cpp
)

# Header files
set(HEADERS
    src/mainwindow.h
    src/videowidget_qt.h
    src/mediawidget.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link Qt6 libraries
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
)

# Set platform-specific target properties
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
elseif(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
    )
endif()

# Copy Qt DLLs if needed (Windows)
if(WIN32)
    get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_WINDEPLOYQT_EXECUTABLE ${QT_QMAKE_EXECUTABLE} PATH)
    set(QT_WINDEPLOYQT_EXECUTABLE "${QT_WINDEPLOYQT_EXECUTABLE}/windeployqt.exe")

    if(EXISTS ${QT_WINDEPLOYQT_EXECUTABLE})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${QT_WINDEPLOYQT_EXECUTABLE} --no-translations --no-system-d3d-compiler $<TARGET_FILE:${PROJECT_NAME}>
            COMMENT "Deploying Qt libraries (excluding translations)"
        )
    endif()
endif()

# Print configuration info
message(STATUS "Qt6 found at: ${CMAKE_PREFIX_PATH}")
message(STATUS "Using Qt6 built-in media player")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
