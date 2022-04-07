![Dawn's logo: a sun rising behind a stylized mountain inspired by the WebGPU logo. The text "Dawn" is written below it.](docs/imgs/dawn_logo.png "Dawn's logo")

# Dawn, a WebGPU implementation

Dawn is an open-source and cross-platform implementation of the work-in-progress [WebGPU](https://webgpu.dev) standard.
More precisely it implements [`webgpu.h`](https://github.com/webgpu-native/webgpu-headers/blob/master/webgpu.h) that is a one-to-one mapping with the WebGPU IDL.
Dawn is meant to be integrated as part of a larger system and is the underlying implementation of WebGPU in Chromium.

Dawn provides several WebGPU building blocks:
 - **WebGPU C/C++ headers** that applications and other building blocks use.
   - The `webgpu.h` version that Dawn implements.
   - A C++ wrapper for the `webgpu.h`.
 - **A "native" implementation of WebGPU** using platforms' GPU APIs:
   - **D3D12** on Windows 10
   - **Metal** on macOS and iOS
   - **Vulkan** on Windows, Linux, ChromeOS, Android and Fuchsia
   - OpenGL as best effort where available
 - **A client-server implementation of WebGPU** for applications that are in a sandbox without access to native drivers

Helpful links:

 - [Dawn's bug tracker](https://bugs.chromium.org/p/dawn/issues/entry) if you find issues with Dawn.
 - [Dawn's mailing list](https://groups.google.com/forum/#!members/dawn-graphics) for other discussions related to Dawn.
 - [Dawn's source code](https://dawn.googlesource.com/dawn)
 - [Dawn's Matrix chatroom](https://matrix.to/#/#webgpu-dawn:matrix.org) for live discussion around contributing or using Dawn.
 - [WebGPU's Matrix chatroom](https://matrix.to/#/#WebGPU:matrix.org)

## Documentation table of content

Developer documentation:

 - [Dawn overview](docs/dawn/overview.md)
 - [Building Dawn](docs/dawn/building.md)
 - [Contributing to Dawn](docs/dawn/contributing.md)
 - [Testing Dawn](docs/dawn/testing.md)
 - [Debugging Dawn](docs/dawn/debugging.md)
 - [Dawn's infrastructure](docs/dawn/infra.md)
 - [Dawn errors](docs/dawn/errors.md)

User documentation: (TODO, figure out what overlaps with the webgpu.h docs)

## Status

(TODO)

## License

Apache 2.0 Public License, please see [LICENSE](/LICENSE).

## Disclaimer

This is not an officially supported Google product.

# Tint

Tint is a compiler for the WebGPU Shader Language (WGSL).

This is not an officially supported Google product.

## Requirements
 * Git
 * CMake (3.10.2 or later)
 * Ninja (or other build tool)
 * Python, for fetching dependencies
 * [depot_tools] in your path

## Build options
 * `TINT_BUILD_SPV_READER` : enable the SPIR-V input reader (off by default)
 * `TINT_BUILD_WGSL_READER` : enable the WGSL input reader (on by default)
 * `TINT_BUILD_SPV_WRITER` : enable the SPIR-V output writer (on by default)
 * `TINT_BUILD_WGSL_WRITER` : enable the WGSL output writer (on by default)
 * `TINT_BUILD_FUZZERS` : enable building fuzzzers (off by default)

## Building
Tint uses Chromium dependency management so you need to install [depot_tools]
and add it to your PATH.

[depot_tools]: http://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up

### Getting source & dependencies

```sh
# Clone the repo as "tint"
git clone https://dawn.googlesource.com/tint tint
cd tint

# Bootstrap the gclient configuration
cp scripts/standalone.gclient .gclient

# Fetch external dependencies and toolchains with gclient
gclient sync
```

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

### Fuzzers on MacOS
If you are attempting fuzz, using `TINT_BUILD_FUZZERS=ON`, the version of llvm
in the XCode SDK does not have the needed libfuzzer functionality included.

The build error that you will see from using the XCode SDK will look something
like this:
```
ld: file not found:/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/11.0.0/lib/darwin/libclang_rt.fuzzer_osx.a
```

The solution to this problem is to use a full version llvm, like what you would
get via homebrew, `brew install llvm`, and use something like `CC=<path to full
clang> cmake ..` to setup a build using that toolchain.

### Checking [chromium-style] issues in CMake builds
The gn based work flow uses the Chromium toolchain for building in anticipation
of integration of Tint into Chromium based projects. This toolchain has
additional plugins for checking for style issues, which are marked with
[chromium-style] in log messages. This means that this toolchain is more strict
then the default clang toolchain.

In the future we will have a CQ that will build this work flow and flag issues
automatically. Until that is in place, to avoid causing breakages you can run
the [chromium-style] checks using the CMake based work flows. This requires
setting `CC` to the version of clang checked out by `gclient sync` and setting
the `TINT_CHECK_CHROMIUM_STYLE` to `ON`.

```sh
mkdir -p out/style
cd out/style
cmake ../..
CC=../../third_party/llvm-build/Release+Asserts/bin/clang cmake -DTINT_CHECK_CHROMIUM_STYLE=ON ../../ # add -GNinja for ninja builds
```

## Issues
Please file any issues or feature requests at
https://bugs.chromium.org/p/tint/issues/entry

## Contributing
Please see the CONTRIBUTING and CODE_OF_CONDUCT files on how to contribute to
Tint.

Tint has a process for supporting [experimental extensions](docs/tint/experimental_extensions.md).
