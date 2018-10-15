# Flat-Async

[![Build Status](https://travis-ci.org/malirod/flat-async.svg?branch=master)](https://travis-ci.org/malirod/flat-async)

Framework which allows to write async code in flat manner.

Simple echo server included to demonstrate usage.

Inspired and based on:

1. [Asynchronous Programming: Back to the Future](http://kukuruku.co/hub/cpp/asynchronous-programming-back-to-the-future).
2. [Asynchronous Programming Part 2: Teleportation through Portals](http://kukuruku.co/hub/cpp/asynchronous-programming-part-2-teleportation-through-portals).

## Platform

Ubuntu 18.04: Clang 7.0, GCC 7.3.0, Cmake 3.10, Conan

C++14 Standard is used.

See `tools/Dockerfile-dev-base` for details how to setup development environment

## Setup

Assuming all further commands are executed from project root.

### Initial setup (post clone)

Get sub-modules with the following command

`git submodule update --init --recursive`

#### Setup git hook

Run `tools/install_hooks.py`

This will allow to perform some code checks locally before posting changes to server.

### Dependencies

Project uses [Conan Package Manager](https://github.com/conan-io/conan)

Install conan with

`sudo -H pip install conan`

CMake will try to automatically setup dependencies.

Add additional repositories to conan:

- `conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan`

Cmake will automatically check required dependencies and setup them taking into account current compiler (clang or gcc).

Dependencies can be setup using custom profile with following command (run from build dir)

`conan install .. <settings> --build missing`

e.g.

`conan install .. -s arch=x86_64 -s os=Linux -s compiler=clang -s compiler.version=7.0 -s compiler.libcxx=libstdc++11 -s build_type=Release -e CXX=clang++ -e CC=clang  --build missing`

**Hint:** to upload build packages to server use the following commands

```bash
conan remote add <REMOTE> https://api.bintray.com/conan/malirod/stable
conan user -p <APIKEY> -r <REMOTE> <USERNAME>
conan install . -r <REMOTE>
conan upload "*" -r <REMOTE> --all
```

## Install pylint - python checker

`sudo pip install pylint`

## Install doxygen

`sudo apt install -y doxygen graphviz`

## Build

### Build commands

By default used clang compiler and debug mode.

Run in project root to build debug version with clang

`mkdir build-clang-debug && cd build-clang-debug && cmake .. && make -j$(nproc)`

To build release version with gcc run the following command

`RUN mkdir build-gcc-release && cd build-gcc-release && cmake -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release .. && make -j$(nproc)`

### Build with sanitizers (clang)

You can enable sanitizers with `SANITIZE_ADDRESS`, `SANITIZE_MEMORY`, `SANITIZE_THREAD` or `SANITIZE_UNDEFINED` options in your CMake configuration. You can do this by passing e.g. `-DSANITIZE_ADDRESS=On` in your command line.

## Run

Run from build directory

`ctest`

or

`bin/testrunner`

## Coverage report

To enable coverage support in general, you have to enable `ENABLE_COVERAGE` option in your CMake configuration. You can do this by passing `-DENABLE_COVERAGE=On` on your command line or with your graphical interface.

If coverage is supported by your compiler, the specified targets will be build with coverage support. If your compiler has no coverage capabilities (I assume Intel compiler doesn't) you'll get a warning but CMake will continue processing and coverage will simply just be ignored.

Collect coverage in Debug mode. Tested with gcc 5.0 and clang 5.0 compiler.

### Sample commands to get coverage html report

```bash
CXX=g++ cmake -DENABLE_COVERAGE=On -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
make test
make lcov-capture
make lcov-genhtml
xdg-open lcov/html/selected_targets/index.html

```

## Integration

`Dockerfile-initial` creates build environment from the scratch. It should be built manually and pushed to DockerHub

`Dockerfile-travis` is used by Travis. It's based on pre-built image from `Dockerfile-initial` on DockerHub

### Create docker image

**Dockerfile-dev-base**: base image, which contains basic environment setup (compiler, build tools)

`docker build -t cpp-dev-base -f tools/Dockerfile-dev-base .`

**Dockerfile-initial**: initial project image, which contains pre-build sources. Based on **Dockerfile-dev-base**

`docker build -t travis-build-cpputils -f tools/Dockerfile-initial .`

Steps to prepare image for Travis

```bash
docker build -t cpp-dev-base -f tools/Dockerfile-dev-base .
docker tag cpp-dev-base $DOCKER_ID_USER/cpp-dev-base
docker build -t travis-build-flat-async -f tools/Dockerfile-initial .
docker tag travis-build-flat-async $DOCKER_ID_USER/travis-build-flat-async
docker login
docker push $DOCKER_ID_USER/cpp-dev-base
docker push $DOCKER_ID_USER/travis-build-flat-async
```

### Clang static analyzer

Sample command to run analyzer. By default report is stored in `/tmp/scan-build*`

```bash
mkdir build-debug
cd build-debug
scan-build --use-analyzer=/usr/bin/clang++ cmake ..
scan-build --use-analyzer=/usr/bin/clang++ make -j$(nproc)
```

or

```bash
cmake ..
make clang-static-analyzer
```

### Clang-tidy

Setting are stored in `.clang-tidy`.

Run

```bash
mkdir build
cd build
cmake ..
make clang-tidy
```

### Documentation

Code contains doxygen. To generate html documentation run command `make doc`. Output will be in `<build dir>\doc\html`
