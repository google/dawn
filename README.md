# Tint

Tint is a compiler for the WebGPU Shader Language (WGSL).

This is not an officially supported Google product.

## Requirements
 * Git
 * CMake (3.10.2 or later)
 * Ninja (or other build tool)
 * Python, for fetching dependencies

## Build options
 * `TINT_BUILD_SPV_PARSER` : enable the SPIR-V input parser

## Building
Tint uses Chromium dependency management so you need to [install depot_tools] and add it to your PATH.

[install depot_tools]: http://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up

### Getting source & dependencies

```sh
# Clone the repo as "tint"
git clone https://dawn.googlesource.com/tint tint && cd tint

# Bootstrap the gclient configuration
cp standalone.gclient .gclient

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

## Issues
Please file any issues or feature requests at
https://bugs.chromium.org/p/tint/issues/entry

## Contributing
Please see the CONTRIBUTING and CODE_OF_CONDUCT files on how to contribute to
Tint.
