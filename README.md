# TableEngine

## Overview 

A File explorer. it worked on windows with Qt.

TableEngine is an open-source project. 
I'm user of FD at MS-DOS and 卓駆☆ at Windows. but both are not compatible of windows 10. so I had started develop same software with few new feature.

## Dependency

* Qt 6.11 or later
* Microsoft Visual C++ 2022
* CMake 3.19 or later
* libarchive — managed by vcpkg
* gtest / gmock (unit test only) — managed by vcpkg
* valijson — git submodule (`support/valijson`)
* QHexView — git submodule (`support/QHexView`)
* ICU libs (bundled with Qt)

## Folder tree

```
  + main            : provide main() function.
  + src             : TableEngine main program.
  + tests           : unit tests.
  + support         : third-party dependencies.
    + vcpkg         :  vcpkg package manager (git submodule).
    + QHexView      :  QHexView widget (git submodule, v5.1.0).
    + valijson      :  valijson header-only library (git submodule, v1.1.0).
  + support_package : valijson headers (legacy location, kept for reference).
```

## How to Build

### Prerequisites

- Qt 6.11 or later — install the **MSVC 2022 64-bit** kit
- CMake 3.19 or later
- Microsoft Visual C++ 2022
- Git (for submodule initialization)

### Steps

**1. Clone and initialize submodules**

```bat
git clone <repo-url> TableEngine
cd TableEngine
git submodule update --init --recursive
```

**2. Bootstrap vcpkg** (first time only)

```bat
support\vcpkg\bootstrap-vcpkg.bat -disableMetrics
```

**3. Configure and build** (Qt Ninja kit — recommended)

```bat
cmake --preset Qt-MSVC2022-amd64-Ninja
cmake --build out/build/Qt-MSVC2022-amd64-Ninja --config Release
```

vcpkg will automatically download and build `libarchive` and `gtest` on the first configure run.

**VS Code**: Open the folder and select the `Qt MSVC2022 64bit - Ninja Multi-Config` configure preset in CMake Tools.

## Usage

https://takashiK.github.io/TableEngine/

## Licence

This software is released under the GPLv2 License, see LICENCE.

## Authors

Takashi Kuwabara.