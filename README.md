# TableEngine

## Overview 

A File explorer. it worked on windows with Qt.

TableEngine is an open-source project. 
I'm user of FD at MS-DOS and 卓駆☆ at Windows. but both are not compatible of windows 10. so I had started develop same software with few new feature.

## Dependency

* Qt ( greater than 5.12 )
* Microsoft Visual C++ 2017
* google mock ( for unit test only )

## Folder tree

```
  + main            : provide main() function.
  + src             : TableEngine main program.
  + tests           : unit tests.
  + support_package : place for support library.
    + include       :  headers of library.
    + debug         :  static and dynamic library for debug.
    + release       :  static and dynamic library for release.
    + license       :  license information for libraries.
```

## How to Build

1. call "qmake -tp vc -recursive" on this directory.
1. open "TableEngine.sln" by Visual Studio

## Usage

This software is developping yet. so it not working now.

## Licence

This software is released under the GPLv2 License, see LICENCE.

## Authors

Takashi Kuwabara.