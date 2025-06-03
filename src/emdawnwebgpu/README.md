# "emdawnwebgpu" (Dawn's fork of Emscripten's WebGPU bindings)

"emdawnwebgpu" is Dawn's fork of the Emscripten WebGPU bindings
(`library_webgpu.js` and friends). The forked files live in
[`//third_party/emdawnwebgpu`](../third_party/emdawnwebgpu/)
and the build targets in this directory produce the other files needed to build
an Emscripten-based project using these bindings.

We keep the `webgpu.h` interface roughly in sync between Dawn and emdawnwebgpu,
however we don't guarantee it will always be in sync - we don't have any
automated testing for this, so we'll periodically fix
it up as needed for import into other projects that use these bindings.

Projects should use this fork (by compiling Dawn as instructed below) if they
want the latest version, which is mostly compatible with the same version of Dawn
Native. For the future of this fork, please see <https://crbug.com/371024051>.

## Using emdawnwebgpu pre-built releases

Pre-built releases are published at <https://github.com/google/dawn/releases>.

**See the [included README](./pkg/README.md) on how to use them.**

TODO(crbug.com/371024051): Link to a sample project in that README.

Instructions are provided below on how to build the package yourself, as well as
samples that exercise the bindings.

If for any reason you don't want to use the package, it's also possible to
build emdawnwebgpu as a subproject of a CMake or gn project.

## Building emdawnwebgpu and emdawnwebgpu_pkg

First, get the Dawn code and its dependencies.
See [building.md](../../docs/building.md).

To build the package, you'll build Dawn's `emdawnwebgpu_pkg` target using
Emscripten. `out/yourbuild/emdawnwebgpu_pkg` combines files from:
- `src/emdawnwebgpu`
- `third_party/emdawnwebgpu`
- `out/yourbuild/gen`

### Set up Emscripten

Get an emsdk toolchain (at least Emscripten 4.0.3, which includes the necessary
tools in the package release). There are two options to do this:

- Set the `dawn_wasm` gclient variable (use
  [`standalone-with-wasm.gclient`](../../scripts/standalone-with-wasm.gclient)
  as your `.gclient`), and `gclient sync`.
  This installs emsdk in `//third_party/emsdk`.
- Install it manually following the official
  [instructions](https://emscripten.org/docs/getting_started/downloads.html#installation-instructions-using-the-emsdk-recommended).

### Standalone with CMake

Set up the build directory using emcmake:

```sh
mkdir out/cmake-wasm
cd out/cmake-wasm

path/to/emsdk/upstream/emscripten/emcmake cmake ../..

# Package
make -j8 emdawnwebgpu_pkg
```

Samples and tests:

```sh
# Samples (for a list of samples, see ENABLE_EMSCRIPTEN targets in src/dawn/samples/CMakeLists.txt)
make -j8 HelloTriangle

# Tests
make -j8 emdawnwebgpu_tests_asyncify emdawnwebgpu_tests_jspi
```

(To use Ninja instead of Make, for better parallelism, add `-GNinja` to the
`cmake` invocation, and build using `ninja`.)

Samples and tests produce HTML files which can be served and viewed in a compatible browser.

### Standalone with GN

- Set up Emscripten as per instructions above using `dawn_wasm`.

- Build the `emdawnwebgpu` and `samples` GN build targets.

Samples and tests produce HTML files in `out/<dir>/wasm` which can be served and viewed in a compatible browser.
