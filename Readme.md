# Ultimate Hindsight


## Building on Windows

```sh
git clone .. && cd ultimate-hindsight
mkdir build && cd build
set Qt5_DIR=C:\Qt\Qt5.12.10\5.12.10\msvc2017_64
cmake -A x64 -DCMAKE_BUILD_TYPE=<Debug|Release> -DCMAKE_INSTALL_PREFIX:PATH="path\to\install\dir" ..\
cmake --build . --config <Debug|Release> --target install
```
