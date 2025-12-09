# Building Dawn

Generally, Dawn's tooling and system dependencies are set by Chromium.
Therefore Chromium's build environment, provided by `GN` and `depot_tools`,
is Dawn's primary build environment. However, Dawn can be built as a
standalone C++ project, often with alternative build generators and C++
compilers, as described below.

## System requirements

 * Git
 * `depot_tools` in your path
 * A build generator, one of:
    * `GN`, provided by `depot_tools`
    * `CMake` 3.16 or later
 * Ninja 1.12 (or another build tool supported by CMake)
 * Python 3, for fetching dependencies
 * Go 1.23 or later: https://go.dev/
 * A C++20 compiler
    * When using GN, the default compiler will be provided automatically.
    * Otherwise, check the platform details below.

- Linux
  - CPU:
    - x86-64 (also known as amd64)
    - arm64
    - x86: untested
    - mips, mips64: untested
  - The `pkg-config` command:
    ```sh
    # Install pkg-config on Ubuntu
    sudo apt-get install pkg-config
    ```
  - C++20 compiler:
    - Clang 19 or later. This the primary supported compiler.
    - GCC 12 or later

- Mac
  - CPU:
    - Apple Silicon
    - Intel x86-64
  - [Xcode](https://developer.apple.com/xcode/) 12.2+.
  - The macOS 12.0 SDK. Run `xcode-select` to check whether you have it.
    ```sh
    ls `xcode-select -p`/SDKs
    ```

- Windows
  - CPU:
    - x86-64 (also known as amd64)
    - arm64 support is experimental
  - Windows SDK 10.0.22621.x or a later version.
  - C++20 compiler:
    - Clang, as provided by GN. This is the primary supported compiler.
    - MSVC 19.41, or Visual Studio 2022 v17.11 or later.
      Note: Dawn is tested daily with MSVC 19.43 or VS 2022 v17.13


## Get the code and its dependencies

### Using `depot_tools`

Dawn uses the Chromium build system and dependency management so you need to [install depot_tools] and add it to the PATH.

[install depot_tools]: http://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up

```sh
# Clone the repo as "dawn"
git clone https://dawn.googlesource.com/dawn dawn && cd dawn

# Bootstrap the gclient configuration
cp scripts/standalone.gclient .gclient

# Fetch external dependencies and toolchains with gclient
gclient sync
```

### Without `depot_tools`

If you cannot or do not want to depend on `depot_tools`, you may use the `tools/fetch_dawn_dependencies.py` to clone the dependencies' repositories:

```sh
# Clone the repo as "dawn"
git clone https://dawn.googlesource.com/dawn dawn && cd dawn

# Fetch dependencies (loosely equivalent to gclient sync)
python tools/fetch_dawn_dependencies.py
```

Use `python tools/fetch_dawn_dependencies.py -h` to know more about the available options.
Contrary to `depot_tools`, this scripts does not figure out option-dependent requirements automatically.

### Linux dependencies

The following packages are needed to build Dawn. (Package names are the Ubuntu names).

* `libxrandr-dev`
* `libxinerama-dev`
* `libxcursor-dev`
* `mesa-common-dev`
* `libx11-xcb-dev`
* `pkg-config`
* `nodejs`
* `npm`

```sh
sudo apt-get install libxrandr-dev libxinerama-dev libxcursor-dev mesa-common-dev libx11-xcb-dev pkg-config nodejs npm
```

Note, `nodejs` and `npm` are only needed if building `dawn.node`.


## Build Dawn

### Compiling using CMake + Ninja
```sh
mkdir -p out/Debug
cd out/Debug
cmake -GNinja ../..
ninja # or autoninja
```

### Compiling using CMake + make
```sh
mkdir -p out/Debug
cd out/Debug
cmake ../..
make # -j N for N-way parallel build
```

### Compiling using gn + ninja
```sh
mkdir -p out/Debug
gn gen out/Debug
autoninja -C out/Debug
```

The most common GN build option is `is_debug=true/false`; otherwise
`gn args out/Debug --list` shows all the possible options.

On macOS you'll want to add the `use_system_xcode=true` in most cases.
(and if you're a googler please get XCode from go/xcode).

To generate a Microsoft Visual Studio solution, add `ide=vs2022` and
`ninja-executable=<dawn parent directory>\dawn\third_party\ninja\ninja.exe`.
The .sln file will be created in the output directory specified.

For CMake builds, you can enable installation with the `DAWN_ENABLE_INSTALL` option
and use `find_package(Dawn)` in your CMake project to discover Dawn and link with
the `dawn::webgpu_dawn` target. Please see [Quickstart with CMake](./quickstart-cmake.md)
for step-by-step instructions.

### Using ccache for CMake builds
There is a substantial number of source files that are needed to be
built for Dawn and its dependencies (~thousands), which can lead to
long compile times on resource bound machines (i.e. laptops),
especially when performing clean builds. The general advice for this
situation is use the gn build with a remote compile system like siso,
if available.

An alternative if remote compilation isn't available, or if using
CMake is essential for your workflow, is to use
[ccache](https://ccache.dev/) to avoid rebuilding targets. For full
details on using ccache see the project's documentation, but below is
an example of how to use it on a Debian based Linux dev env.

```sh
# One time Install and setup of ccache
apt-get install ccache
ccache -F 0 -M 0  # Sets the number of files and size of the cache to unlimited, the default is 5 GiB. Choose values that make sense for your situation.

# Setup CMake + Ninja building using cccache
mkdir -p out/cache
cd out/cache
cmake -GNinja -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache <Any other CMake flags> ../.
ninja # or autoninja

# Optional: In another terminal, monitor the status of the cache. (Will have a lot of misses on the first run)
watch ccache -s
```

Using ccache will not help with the initial build of Dawn, since the
targets still need to be built at least once locally, but subsequent
builds, even after a clean operation should be much faster. Some
operations like linking are not cache-able, so will need to be run on
every build, but the bulk of file compilations should be
cache-able. Updating the repo, editing source files, and changing build
flags will all cause misses, since entries in the cache are based on
flags + contents of the source file.

### Fuzzers on MacOS
As of Late Oct 2025, fuzzing on a dev Mac is not in a good state.

The old workaround for fuzzing with XCode 16.X should still work, but that is
not the Chromium supported version of XCode so there may be other issues. That
workaround does not work for XCode 26.X, which is supported by Chromium.

If you attempt to build and run the fuzzers without a workaround, you will
either get build failures like this:
```sh
ld: file not found:/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/11.0.0/lib/darwin/libclang_rt.fuzzer_osx.a
```

or if using a component build, you will get a similar runtime issue for a
missing `.dylib`.

The root cause for this is that the build of llvm shipped as part of the XCode
SDK does not include the standard support libraries for fuzzing.

The workaround for this depends on which version of XCode you are trying to use.

#### Using XCode 16.X
The workaround for 16.X has been to install a fully featured version llvm onto
your system, for example via Homebrew, `brew install llvm`, (you might have to
use `llvm@XX` where `XX` is the major version of llvm in the XCode SDK). And
then using environment variables, i.e. `CC=<path to Homebrew clang> cmake
-GNinja ../..`, to set up the build including the full llvm toolchain to get the
missing libraries onto the search path. This will actually create a hybrid
toolchain using some elements of the XCode SDK and the llvm SDK, because there
is some Apple specific framework stuff that isn't in mainstream llvm.

#### Using XCode 26.X
As mentioned above, the 16.X workaround appears to no longer work due to drift
between Apple's llvm and the mainline version. Trying to create a hybrid
toolchain will lead to compiling issues from the XCode standard headers using
deprecated functions according to mainstream llvm, even when using the same
major version.

For 26.X there is workaround done by installing `llvm@17` (since AppleClang is
version 17) from Homebrew, and then copying over the missing library files
(i.e. `libclang_rt.fuzzer_osx.a`) from the Homebrew directory to the
CommandLineTools toolchain directory. (Cannot be the XCode toolchain directory,
because that is write protected).

You will then need to be using the CLT toolchain as your active toolchain
```sh
sudo xcode-select -s /Library/Developer/CommandLineTools
```

This allows for building and debugging the fuzzers, but the impact of this on
other builds and/or system stability is unknown. It shouldn't have a significant
impact, since the standard SDK doesn't ship with these libraries, so nothing
else should be using them, but the Homebrew version of llvm will not have all of
Apple's patches, so may be incompatible in subtle ways if these libraries do get
used somehow.

#### Using hermetic builds
It should be possible to use the same hermetic toolchain that the bots
use for dev builds of the fuzzers, since the bots build and run the fuzzer fine.

Unfortunately the standard setup on a dev machine is 'mostly' hermetic. It does
use the hermetic version clang and other tools, but it also does use some of the
system framework stuff that comes from XCode, and that is where the build issues
come in from, when trying to build/run the fuzzers.

There is some support for getting a truly hermetic build on a dev machine,
https://source.chromium.org/chromium/chromium/src/+/main:third_party/dawn/src/cmake/HermeticXcode/,
but that has not been tested with building the fuzzers, and probably needs work
to be a drop-in solution here.

### Reproducing bot specific environments on Windows + CMake
When investigating build issues being seen by CI/CQ it is sometimes necessary
to replicate the exact environment from a builder/bot for local debugging.

As part of a Dawn checkout the toolchains used by the builders should already be
checked out in your repo. For Windows, using CMake, to use them instead of the
system installed toolchain, there is a script supplied to set up the
environment.

Instead of using the system batch file to set up the environment, i.e.
```sh
C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat
```
use the appropriate script from `third_party\depot_tools\win_toolchain`, i.e.
```sh
 C:\src\dawn\third_party\depot_tools\win_toolchain\vs_files\68a20d6dee\Windows Kits\10\bin\SetEnv.cmd
```
