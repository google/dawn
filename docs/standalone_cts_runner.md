# Running the WebGPU CTS with the standalone runner

`dawn_standalone_cts_runner` is a single executable that statically links V8 and
Dawn and runs the WebGPU CTS directly, without a Node.js installation.

The usual way to run the CTS is [`dawn.node`](../src/dawn/node/README.md), a
native addon loaded by Node.js. The standalone runner instead embeds its own V8
and supplies the subset of the Node API the CTS uses. That removes the external
Node.js dependency. This is important for platforms like Android where Node.js
is not available.

Both runners execute the same CTS JavaScript and the same Dawn bindings, and
both are driven by [`run-cts`](../tools/src/cmd/run-cts): `run-cts standalone`
is to the standalone runner what `run-cts node` is to `dawn.node`.

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

## Running the CTS

`run-cts standalone` builds the CTS JavaScript when it is out of date, expands
the query into its test cases, runs them in batches through runner processes,
and reports the results the way `run-cts` does for `dawn.node`. It needs the
CTS dependencies installed once:

```sh
cd third_party/webgpu-cts && npm ci && cd ../..
```

`npm` and `npx` come with any Node.js installation (`>= 16.0.0`). If you have
none, add the Node.js fetched by `gclient sync` to your `PATH` first:

```sh
# Replace linux-amd64 with linux-arm64, mac-amd64, mac-arm64 or windows-amd64.
export PATH="$PWD/third_party/node/linux-amd64/bin:$PATH"
```

Then run a query from the Dawn root (on Windows, from Git Bash):

```sh
./tools/run run-cts standalone -bin out/Release 'webgpu:api,operation,queue,*'
```

`-bin` is the directory holding `dawn_standalone_cts_runner` (`-runner <path>`
names the executable itself), and may be omitted when the build directory is
`out/active`. Several queries may be given, after all of the flags. Quote them:
CTS queries contain `;` and `"`, which most shells would otherwise interpret.

Each runner process is given a batch of at most `-batch-size` test cases (500
by default) on its standard input, `-j` processes run at a time, and a short
list is split evenly over them. Each result is reported as soon as the runner
prints it. When a process crashes, or prints no result for `-case-timeout` (one
minute by default), the test case it was running is reported as a crash or a
timeout, with the output it produced, and a new process runs the rest of the
batch (`-recover=false` reports the rest as not run instead).

Flags you are likely to want:

- `-backend <name>`, `-adapter <substring>` and `-validate` select the GPU and
  enable backend validation
- `-flag <key>=<value>` sets any other Dawn flag, and may be repeated; the keys
  are listed under [Dawn flags](#dawn-flags) below
- `-verbose` prints the runner command lines and all of the test output
- `-output <file>` writes the results as JSON, `-expect <file>` compares a run
  with such a file, and `-log <file>` records every test and its result
- `-build=false` skips the CTS build, for example when `npx` is not on `PATH`
- `-cts <dir>` uses a CTS checkout other than `third_party/webgpu-cts`

`./tools/run run-cts standalone -help` lists the rest.

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

### Running

With `-adb`, `run-cts standalone` runs the runner on the device connected
through `adb` (which must be on `PATH`) instead of on the host:

```sh
./tools/run run-cts standalone -adb -bin out/Android \
    'webgpu:api,operation,queue,*'
```

Before the first batch, it copies the runner and a zip of the compiled CTS into
`-device-dir`, `/data/local/tmp/cts` by default, and records their hashes in a
stamp file there, so that later runs copy nothing unless the runner or the CTS
changed. `-stage=always` copies both even if the stamp matches, `-stage=never`
skips the copy, and `-stage-only` copies and exits.
`-adb-serial <serial>` picks the device when several are connected.

Everything else works as on the host: the CTS is built and the query expanded
on the host, each batch runs on the device through `adb shell`, and a hung batch
is killed on the device. `-j` defaults to 1 with `-adb`; raise it to run several
runner processes on the device at once.

## Running the runner by hand

`run-cts` is the convenient way, but the runner needs neither it, nor Go, nor
Node.js. This section is for running `dawn_standalone_cts_runner` directly, for
instance on a machine that only has the executable and the compiled CTS.

### Building the CTS JavaScript

The runner executes the CTS's compiled `out-node/` tree, which is not checked in
and which `run-cts` would otherwise build for you. With `npm` and `npx` on your
`PATH` (see above), install the CTS dependencies and build `out-node/`:

```sh
cd third_party/webgpu-cts
npm ci
npx grunt node
cd ../..
```

`npm ci` alone is not sufficient: `npx grunt node` runs the Grunt tasks that
generate `out-node/`.

### Running

Run from the Dawn root (or pass `--cts-dir <dir>` if the CTS lives outside
`third_party/webgpu-cts`):

```sh
out/Release/dawn_standalone_cts_runner -q 'webgpu:api,operation,queue,*'
```

`-q` may be repeated. `--query-file <file>` reads queries or test cases, one per
line, from a file, or from stdin when the file is `-`, and `--list` prints the
matching test cases instead of running them. Pass `--help` to list all
supported flags.

### Dawn flags

`--dawn-flag` is passed through to the Dawn provider. It takes a single
`key=value` and may be repeated (`run-cts standalone -flag` sets the same keys):

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

### Android by hand

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

Then run it through `adb shell`:

```sh
adb shell '/data/local/tmp/cts/bin/dawn_standalone_cts_runner \
    --cts-dir /data/local/tmp/cts/webgpu-cts \
    -q "webgpu:api,operation,queue,*"'
```
