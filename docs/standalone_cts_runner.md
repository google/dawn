# Running the WebGPU CTS with the standalone runner

`dawn_standalone_cts_runner` is a single executable that statically links V8 and
Dawn and runs the WebGPU CTS directly, without a Node.js installation.

The usual way to run the CTS is [`dawn.node`](../src/dawn/node/README.md), a
native addon loaded by Node.js. The standalone runner instead embeds its own V8
and supplies the subset of the Node API the CTS uses. That removes the external
Node.js dependency. This is important for platforms like Android where Node.js
is not available.

Both runners execute the same CTS JavaScript and the same Dawn bindings.

## Fetching dependencies

Follow [`docs/building.md`](building.md), using
`scripts/standalone-with-v8.gclient` in the `Get the code` step so `gclient`
fetches both the Node bindings (`dawn_node`) and V8 (`checkout_v8`):

```sh
# Clone the repo as "dawn"
git clone https://dawn.googlesource.com/dawn dawn && cd dawn

# Bootstrap the gclient configuration for the standalone CTS runner
cp scripts/standalone-with-v8.gclient .gclient

# Fetch external dependencies and toolchains with gclient
gclient sync
```

If you are configuring an existing checkout, ensure `.gclient` sets both
`"checkout_v8": True` and `"dawn_node": True` in `custom_vars` before running
`gclient sync`. Without `checkout_v8`, the `dawn_standalone_cts_runner` target
is not declared.

## Building the runner

Generate a release build directory and compile `dawn_standalone_cts_runner`
(building this target compiles only the standalone runner executable and its
dependencies, not the `dawn.node` shared library):

```sh
gn gen out/Release --args="is_debug=false dawn_use_swiftshader=false"
autoninja -C out/Release dawn_standalone_cts_runner
```

- `is_debug=false` is strongly recommended: the default debug build compiles a
  debug V8, which runs the CTS roughly 17x slower than a release build.
- `dawn_use_swiftshader=false` skips building Dawn's bundled SwiftShader
  renderer and ensures the runner uses the system's drivers. Remove this option
  if you want to test against SwiftShader.

## Building the CTS JavaScript

The runner executes the CTS's compiled `out-node/` tree, which is not checked
in. Any Node.js installation (`>= 16.0.0`) with `npm` and `npx` can build it. If
you do not have `node`, `npm`, and `npx` installed on your system, you can add
the prebuilt Node.js fetched by `gclient sync` under `third_party/node/` to your
`PATH` first:

```sh
# Optional: use the Node.js fetched by DEPS if npm/npx are not already installed.
# Replace linux-amd64 with linux-arm64, mac-amd64, or mac-arm64 as needed
# (or $PWD/third_party/node/windows-amd64 on Windows).
export PATH="$PWD/third_party/node/linux-amd64/bin:$PATH"
```

Install the CTS dependencies and build `out-node/`:

```sh
cd third_party/webgpu-cts
npm ci
npx grunt node
cd ../..
```

`npm ci` alone is not sufficient: `npx grunt node` runs the Grunt tasks that
generate `out-node/`.

## Running the CTS

Run from the Dawn root (or pass `--cts-dir <dir>` if the CTS lives outside
`third_party/webgpu-cts`):

```sh
out/Release/dawn_standalone_cts_runner -q 'webgpu:api,operation,queue,*'
```

Quote the query: CTS queries contain `;` and `"`, which most shells will
otherwise interpret. Pass `--help` to list all supported flags.

### Selecting a GPU, and setting Dawn toggles

`--dawn-flag` is passed through to the Dawn provider. It takes a single
`key=value` and may be repeated:

```sh
out/Release/dawn_standalone_cts_runner \
    --dawn-flag backend=vulkan \
    --dawn-flag enable-dawn-features=dump_shaders \
    -q 'webgpu:api,operation,queue,*'
```

The recognised keys are:

- `backend=<null|webgpu|d3d11|d3d12|d3d|metal|vulkan|vk|opengl|gl|opengles|gles>`
  (defaults to `d3d12` on Windows, `vulkan` on Linux/Android, and `metal` on
  macOS; can also be set via the `DAWNNODE_BACKEND` environment variable)
- `adapter=<name>` — substring match against the adapter device name. Pass an
  invalid name (for example `adapter=help`) to list all available adapters in
  the error output
- `validate=<1|true>` — enable full backend validation layers
- `verbose=1` — print the selected GPU adapter name
- `enable-dawn-features=<toggles>` — comma-separated list of
  [Dawn toggles](https://dawn.googlesource.com/dawn/+/refs/heads/main/src/dawn/native/Toggles.cpp)
- `disable-dawn-features=<toggles>` — comma-separated list of Dawn toggles

## Running on Android

There is no Node.js build for Android in the checkout, so the standalone runner
is the only way to run the command-line CTS directly on an Android device.
Cross-compiling for Android currently requires a Linux host (the GN build and
Android NDK toolchain in Dawn's checkout do not support building for Android
from Windows or macOS).

### Checkout

On a Linux host, add `target_os = ["android"]` to `.gclient` and sync the
Android NDK and toolchain dependencies:

```sh
printf '\ntarget_os = ["android"]\n' >> .gclient
gclient sync
```

### Build

Generate an `arm64` Android release build on Linux and compile the runner:

```sh
gn gen out/Android --args='target_os="android" target_cpu="arm64" is_debug=false'
autoninja -C out/Android dawn_standalone_cts_runner
```

### Staging onto the device

Only the executable and the compiled CTS JavaScript are needed on the device.

> [!NOTE]
> `dawn.node` is deliberately **not** pushed. The runner's module loader returns
> its statically-linked binding for `require('dawn.node')`, so the shared
> library is never loaded.

```sh
DEV=/data/local/tmp/cts
adb shell "mkdir -p $DEV/bin $DEV/webgpu-cts"

adb push out/Android/dawn_standalone_cts_runner $DEV/bin/
adb shell "chmod 755 $DEV/bin/dawn_standalone_cts_runner"

# out-node is ~2200 files; a single archive transfers far faster than letting
# adb sync each file individually. Include cmdline.ts as well, which both the
# runner and cmdline.js check as a repository-root marker.
tar -C third_party/webgpu-cts -czf /tmp/cts.tgz \
    out-node src/common/runtime/cmdline.ts
adb push /tmp/cts.tgz $DEV/webgpu-cts/
adb shell "cd $DEV/webgpu-cts && tar xzf cts.tgz && rm cts.tgz"
rm /tmp/cts.tgz
```

### Running

```sh
adb shell '/data/local/tmp/cts/bin/dawn_standalone_cts_runner \
    --cts-dir /data/local/tmp/cts/webgpu-cts \
    -q "webgpu:api,operation,queue,*"'
```

## Future plans

The setup above is more manual than it should be. The intent is to teach
[`run-cts`](../tools/src/cmd/run-cts) about the standalone runner, so that
selecting it becomes a flag on the tool already used to run the CTS, with the
CTS build, query sharding, expectations handling and result reporting all
handled as they are today for `dawn.node`.

Once that lands, most of this document reduces to a single `./tools/run run-cts`
invocation, and the manual staging steps for Android are expected to be driven
by the tool as well.
