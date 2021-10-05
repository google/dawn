# Dawn bindings for NodeJS

Note: This code is currently WIP. There are a number of [known issues](#known-issues).

## Building

## System requirements

- [CMake 3.10](https://cmake.org/download/) or greater
- [Go 1.13](https://golang.org/dl/) or greater

## Install `depot_tools`

Dawn uses the Chromium build system and dependency management so you need to [install depot_tools] and add it to the PATH.

[install depot_tools]: http://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up

### Fetch dependencies

First, the steps are similar to [`doc/building.md`](../../docs/building.md), but instead of the `Get the code` step, run:

```sh
# Clone the repo as "dawn"
git clone https://dawn.googlesource.com/dawn dawn && cd dawn

# Bootstrap the NodeJS binding gclient configuration
cp scripts/standalone-with-node.gclient .gclient

# Fetch external dependencies and toolchains with gclient
gclient sync
```

### Build

Currently, the node bindings can only be built with CMake:

```sh
mkdir <build-output-path>
cd <build-output-path>
cmake <dawn-root-path> -GNinja -DDAWN_BUILD_NODE_BINDINGS=1 -DDAWN_ENABLE_PIC=1
ninja dawn.node
```

### Running WebGPU CTS

1. [Build](#build) the `dawn.node` NodeJS module.
2. Checkout the [WebGPU CTS repo](https://github.com/gpuweb/cts)

```sh
./src/dawn_node/tools/run-cts --cts=<path-to-webgpu-cts> --dawn-node=<path-to-dawn.node> [WebGPU CTS query]
```

If this fails with the error message `TypeError: expander is not a function or its return value is not iterable`, try appending `--build=false` to the start of the `run-cts` command line flags.

To test against SwiftShader instead of the default Vulkan device, prefix `./src/dawn_node/tools/run-cts` with `VK_ICD_FILENAMES=<swiftshader-cmake-build>/Linux/vk_swiftshader_icd.json`

## Known issues

- Many WebGPU CTS tests are currently known to fail
- Dawn uses special token values for some parameters / fields. These are currently passed straight through to dawn from the JavaScript. discussions: [1](https://dawn-review.googlesource.com/c/dawn/+/64907/5/src/dawn_node/binding/Converter.cpp#167), [2](https://dawn-review.googlesource.com/c/dawn/+/64907/5/src/dawn_node/binding/Converter.cpp#928), [3](https://dawn-review.googlesource.com/c/dawn/+/64909/4/src/dawn_node/binding/GPUTexture.cpp#42)
- Backend validation is currently always set to 'full' to aid in debugging. This can be extremely slow. [discussion](https://dawn-review.googlesource.com/c/dawn/+/64916/4/src/dawn_node/binding/GPU.cpp#25)
- Attempting to call `new T` in JavaScript, where `T` is an IDL interface type, should result in a TypeError "Illegal constructor". [discussion](https://dawn-review.googlesource.com/c/dawn/+/64902/9/src/dawn_node/interop/WebGPU.cpp.tmpl#293)
- `GPUDevice` currently maintains a list of "lost promises". This should return the same promise. [discussion](https://dawn-review.googlesource.com/c/dawn/+/64906/6/src/dawn_node/binding/GPUDevice.h#107)

## Remaining work

- Investigate CTS failures that are not expected to fail.
- Generated includes live in `src/` for `dawn_node`, but outside for Dawn. [discussion](https://dawn-review.googlesource.com/c/dawn/+/64903/9/src/dawn_node/interop/CMakeLists.txt#56)
- Hook up to presubmit bots (CQ / Kokoro)
- `binding::GPU` will require significant rework [once Dawn implements the device / adapter creation path properly](https://dawn-review.googlesource.com/c/dawn/+/64916/4/src/dawn_node/binding/GPU.cpp).
