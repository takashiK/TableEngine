# TableEngine

## Overview 

A File explorer. it worked on windows with Qt.

TableEngine is an open-source project. 
I'm user of FD at MS-DOS and 卓駆☆ at Windows. but both are not compatible of windows 10. so I had started develop same software with few new feature.

## Dependency

* Qt ( greater than 6.10 )
* Microsoft Visual C++ 2022
* google mock ( for unit test only )
* libmagic
* libarchive
* valijson
* ICU libs

## Folder tree

```
  + main            : provide main() function.
  + src             : TableEngine main program.
  + tests           : unit tests.
  + support_package : place for support library.
    + include       :  headers of library.
      + gmock        :  google mock headers.
      + gtest        :  google test headers.
      + valijson      :  valijson headers.
      - archive.h
      - archive_entry.h
      - magic.h
    + debug         :  static and dynamic library for debug.
      - archive.dll
      - archive.lib
      - archive.exp
      - gmock_maind.lib
      - gmockd.lib
      - gtest_maind.lib
      - gtestd.lib
      - libmagic.dll
      - libmagic.lib
      - libmagic.exp
      - magic.mgc
    + release       :  static and dynamic library for release.
      - archive.dll
      - archive.lib
      - archive.exp
      - gmock_main.lib
      - gmock.lib
      - gtest_main.lib
      - gtest.lib
      - libmagic.dll
      - libmagic.lib
      - libmagic.exp
      - magic.mgc
```

## How to Build

### Prerequisites

- Qt 6.10 or later (with `Qt6_DIR` or `CMAKE_PREFIX_PATH` pointing to the Qt installation)
- CMake 3.19 or later
- Microsoft Visual C++ 2022

### Steps

```bat
cmake --preset "Visual Studio Community 2022 Release - amd64"
cmake --build out/build/"Visual Studio Community 2022 Release - amd64" --config Release
```

Alternatively, open the generated solution in Visual Studio:

```bat
cmake --preset "Visual Studio Community 2022 Release - amd64"
start out/build/"Visual Studio Community 2022 Release - amd64"/TableEngine.sln
```

Please build the support libraries yourself and copy them into the `support_package` directory (or install them via a package manager).

## Usage

This software is developping yet. so it not working now.

## Licence

This software is released under the GPLv2 License, see LICENCE.

## Authors

Takashi Kuwabara.