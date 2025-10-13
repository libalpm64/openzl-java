## openzl-java
OpenZL is a lossless data compression framework that provides context awareness unlike zstd, zlib, etc. It uses a composed DAG to create custom compressors for specific data formats.

> These JNI bindings do not access OpenZL's current format-aware compression methods.  
> OpenZL is new and production-ready, but its codebase is still subject to change.  
> It is recommended to avoid interfacing with format-aware APIs for now.  
> You may generate bindings for them if required, but without format awareness, compression will be slightly worse than zstd-3.

### Build Requirements

You must have the following installed:
- CMake
- LLVM (includes Clang, etc.)

Windows ONLY:
- A MinGW-compatible toolchain (e.g., w64devkit / MiniGMW) that provides `windres` and `mingw32-make`

### Build Commands

**Windows**
```sh
cmake -S . -B build ^
  -DCMAKE_C_COMPILER="C:/Program Files/LLVM/bin/clang.exe" ^
  -DCMAKE_CXX_COMPILER="C:/Program Files/LLVM/bin/clang++.exe" ^
  -DCMAKE_RC_COMPILER="C:/path/to/w64devkit/bin/windres.exe" ^
  -DCMAKE_MAKE_PROGRAM="C:/path/to/w64devkit/bin/mingw32-make.exe" ^
  -DOPENZL_BUILD_TOOLS=OFF ^
  -DOPENZL_BUILD_CLI=OFF ^
  -DOPENZL_BUILD_EXAMPLES=OFF ^
  -G "MinGW Makefiles" ^
  --fresh

cmake --build build --config Release
```
> Make sure these are your correct installation paths. You need to install LLVM and CMake.

**Linux**

```sh
cmake -S . -B build \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DOPENZL_BUILD_TOOLS=OFF \
  -DOPENZL_BUILD_CLI=OFF \
  -DOPENZL_BUILD_EXAMPLES=OFF \
  --fresh
```
```sh
cmake --build build --config Release
```

For Java, use a compiler like Maven to compile the repo.
