# TableEngine

## Overview 

A File explorer. it worked on Windows and Ubuntu with Qt.

TableEngine is an open-source project. 
I'm user of FD at MS-DOS and 卓駆☆ at Windows. but both are not compatible of windows 10. so I had started develop same software with few new feature.

## Dependencies

### Windows

* Qt 6.11 or later
* Microsoft Visual C++ 2022
* CMake 3.19 or later
* libarchive and gtest / gmock (unit test only) — managed by vcpkg
* ICU libraries bundled with Qt

### Ubuntu 26.04

Linux builds use Ubuntu's system packages and never use vcpkg. Install the
following packages before configuring the project:

```bash
sudo apt update
sudo apt install -y \
  build-essential cmake ninja-build git pkg-config dpkg-dev \
  qt6-base-dev qt6-base-dev-tools qt6-tools-dev qt6-l10n-tools qt6-qpa-plugins \
  libarchive-dev libicu-dev libgtest-dev libgmock-dev
```

Ubuntu 26.04's Qt 6.10 packages are supported. `valijson`, `QHexView`,
`QLightTerminal`, and `ptyqt` are Git submodules on every platform.

## Folder tree

```
  + main            : provide main() function.
  + src             : TableEngine main program.
    + platform      : OS abstraction layer (Windows / Linux).
  + tests           : unit tests.
  + support         : third-party dependencies.
    + vcpkg         :  vcpkg package manager (git submodule).
    + QHexView      :  QHexView widget (git submodule, v5.1.0).
    + valijson      :  valijson header-only library (git submodule, v1.1.0).
    + QLightTerminal:  QLightTerminal widget (git submodule).
      + ptyqt         :  ptyqt library (git submodule).
```

## How to Build

### Windows Prerequisites

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

### Ubuntu 26.04

After installing the packages above, configure, build, test, and package with
the apt-only presets:

```bash
cmake --preset Ubuntu-GCC-Ninja-release
cmake --build --preset Ubuntu-GCC-Ninja-release
ctest --preset Ubuntu-GCC-Ninja-release
cpack --preset ubuntu-deb
```

The generated DEB is placed in `out/build/Ubuntu-GCC-Ninja-release/`. Inspect
its dependencies and installed paths with `dpkg-deb -I` and `dpkg-deb -c`.

Only CMake builds are supported; qmake (`.pro`) builds are unsupported.

## Usage

https://takashiK.github.io/TableEngine/

## Licence

This software is released under the GPLv2 License, see LICENCE.

## Authors

Takashi Kuwabara.