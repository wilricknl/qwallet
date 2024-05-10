# Build

This wallet uses [vcpkg](https://github.com/microsoft/vcpkg) for packagement - the following instructions give examples on how to get up and running on both Windows and Linux.

### Windows

#### Install vcpkg

```shell
git clone https://github.com/microsoft/vcpkg
cd vcpkg
.\bootstrap-vcpkg.bat -disableMetrics
.\vcpkg integrate install
```

#### Building

This project builds out of the box in Visual Studio 2022 with an integrated installation of vcpkg.

### Linux

#### Install vcpkg

```shell
cd
mkdir tools
cd tools
git clone https://github.com/microsoft/vcpkg
cd vcpkg
./bootstrap-vcpkg.sh -disableMetrics
```

#### Building from the terminal

When building from the terminal CMake needs to point to the vcpkg build system.

```shell
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=~/tools/vcpkg/scripts/buildsystems/vcpkg.cmake ..
make
```

#### Configure vcpkg in CLion

The following CMake settings are required to build this project within CLion.

```
1. Go to `File -> Settings -> Build, Execution, Deployment -> CMake`.
2. Find `CMake Options`.
3. Insert `-DCMAKE_TOOLCHAIN_FILE=~/tools/vcpkg/scripts/buildsystems/vcpkg.cmake`.
```
