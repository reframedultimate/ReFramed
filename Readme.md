# ReFramed

A general frame-data analysis tool for Super Smash Bros. Ultimate.

## Building on Windows

```sh
git clone .. && cd ultimate-hindsight
mkdir build && cd build
set Qt5_DIR=C:\Qt\Qt5.14.1\5.14.1\msvc2017_64
cmake -A x64 -DCMAKE_BUILD_TYPE=<Debug|Release> -DCMAKE_INSTALL_PREFIX:PATH="path\to\install\dir" -DCMAKE_CXX_FLAGS="/MP" ..\
cmake --build . --config <Debug|Release> --target install
```

Explanations:
  - ```Qt5_DIR```: Environment variable cmake uses to find the package config file for Qt5 (Qt5Config.cmake).
  - ```-A x64```: Build 64-bit version. To build 32-bit, you can do ```-A win32```. For older versions of MSVC the ```-A``` option is invalid. In these cases you will have to use one of the available generators (see ```cmake --help```). Example: ```cmake -G "Visual Studio 15 2017 Win64"``` for 64-bit.
  - ```CMAKE_BUILD_TYPE```
