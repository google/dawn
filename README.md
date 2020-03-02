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

```
./tools/git-sync-deps
mkdir -p out/Debug
cd out/Debug
cmake -GNinja ../..
ninja
```

## Contributing
Please see the CONTRIBUTING and CODE_OF_CONDUCT files on how to contribute to
Tint.
