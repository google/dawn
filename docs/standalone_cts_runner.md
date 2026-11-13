# Running the WebGPU CTS with the standalone runner

`dawn_standalone_cts_runner` is a single executable that statically links V8 and
Dawn and runs the WebGPU CTS directly, without a Node.js installation.

The usual way to run the CTS is [`dawn.node`](../src/dawn/node/README.md), a
native addon loaded by Node.js. The standalone runner instead embeds its own V8
and supplies the subset of the Node API the CTS uses, so it also runs where
Node.js is not available, such as Android.

Both runners execute the same CTS JavaScript and the same Dawn bindings, and
both are driven by [`run-cts`](../tools/src/cmd/run-cts): `run-cts standalone`
is to the standalone runner what `run-cts node` is to `dawn.node`.

## Setup

### Fetching dependencies

Follow [`docs/building.md`](building.md), using
`scripts/standalone-with-v8.gclient` in the `Get the code` step so `gclient`
fetches both the Node bindings (`dawn_node`) and V8 (`checkout_v8`):

```sh
git clone https://dawn.googlesource.com/dawn dawn && cd dawn
cp scripts/standalone-with-v8.gclient .gclient
gclient sync
```

In an existing checkout, set `"checkout_v8": True` and `"dawn_node": True` in
the `custom_vars` of `.gclient` instead. Without `checkout_v8`, the
`dawn_standalone_cts_runner` target is not declared.

To build for Android, which needs a Linux host, also fetch the Android NDK:

```sh
printf '\ntarget_os = ["android"]\n' >> .gclient
gclient sync
```

### Building the runner

```sh
gn gen out/Release --args="is_debug=false dawn_use_swiftshader=false"
autoninja -C out/Release dawn_standalone_cts_runner
```

A debug build also builds a debug V8, which makes the CTS many times slower.
Drop `dawn_use_swiftshader=false` to test against SwiftShader.

For an arm64 Android device:

```sh
gn gen out/Android --args='target_os="android" target_cpu="arm64" is_debug=false'
autoninja -C out/Android dawn_standalone_cts_runner
```

## Running with run-cts

`run-cts` is Dawn's Go tool for running the CTS and reporting the results.
`run-cts standalone` builds the CTS JavaScript when it is out of date, expands
the queries into test cases, runs them in batches of runner processes, and
reports the results as `run-cts` does for `dawn.node`. A crash or a hang fails
the test case it happened in, and the run carries on with the next one.

`run-cts` builds the CTS JavaScript with `npx`, which it looks for on your
`PATH`, but it does not install the CTS dependencies. If you do not have Node.js
(`>= 16.0.0`) installed, use the one `gclient sync` fetched, then install the
dependencies once:

```sh
# Replace linux-amd64 with linux-arm64, mac-amd64, mac-arm64 or windows-amd64.
export PATH="$PWD/third_party/node/linux-amd64/bin:$PATH"
cd third_party/webgpu-cts && npm ci && cd ../..
```

Neither Node.js nor the dependencies are needed to run a [CTS
zip](#running-from-a-zip), or with `-build=false` once `out-node/` is built.

Then run queries from the Dawn root (on Windows, from Git Bash):

```sh
./tools/run run-cts standalone -bin out/Release 'webgpu:api,operation,queue,*'
```

`-bin` is the directory holding `dawn_standalone_cts_runner`, and may be omitted
when it is `out/active`. Several queries may follow the flags. Quote them: CTS
queries contain `;` and `"`, which most shells would otherwise interpret.

### On an Android device

With `-adb`, the runner runs on the device connected through `adb`, which must
be on `PATH`:

```sh
./tools/run run-cts standalone -adb -bin out/Android \
    'webgpu:api,operation,queue,*'
```

The runner and the CTS are copied to the device, and copied again only when they
change. Flags for `-adb`:

- `-device-dir <dir>` is where they are copied, `/data/local/tmp/cts` by default
- `-stage=always` forces the copy, `-stage=never` skips it, and `-stage-only`
  copies and exits
- `-adb-serial <serial>` picks the device when several are connected
- `-j` defaults to 1

### Flags

- `-backend <name>`, `-adapter <substring>` and `-validate` select the GPU and
  enable backend validation
- `-flag <key>=<value>` sets any other [Dawn flag](#dawn-flags), and may be
  repeated
- `-verbose` prints the runner command lines and all of the test output
- `-output <file>` writes the results as JSON, `-expect <file>` compares a run
  with such a file, and `-log <file>` records every test and its result
- `-build=false` skips building the CTS JavaScript
- `-shard-count <n>` with `-shard-index <i>` runs only shard `i` of `n` of the
  test cases, from 0 to `n` - 1, so that several machines can each run one
- `-cts <dir>` uses a CTS checkout other than `third_party/webgpu-cts`, and
  `-cts <zip>` a [CTS zip](#running-from-a-zip)

`./tools/run run-cts standalone -help` lists the rest.

## Running from a zip

The CTS JavaScript can also be read straight out of a zip, which is one file to
copy instead of thousands. `run-cts` builds the CTS JavaScript if it is out of
date and zips it:

```sh
./tools/run run-cts zip-cts -o cts.zip
./tools/run run-cts standalone -bin out/Release -cts cts.zip \
    'webgpu:api,operation,queue,*'
```

With a zip, nothing is built, and `-adb` copies the zip to the device as it is.

### On another machine

`run-cts`, the runner and the zip are all that is needed to run the CTS on a
machine without a Dawn checkout, Go or Node.js. `run-cts` can be built for that
machine with the Go toolchain in `tools/golang`. Use the Go for your host, and
set `GOOS` and `GOARCH` to build for another one:

```sh
mkdir -p package
CGO_ENABLED=0 tools/golang/linux-amd64/bin/go build -trimpath \
    -o package/run-cts ./tools/src/cmd/run-cts
CGO_ENABLED=0 GOOS=windows GOARCH=amd64 tools/golang/linux-amd64/bin/go build \
    -trimpath -o package/run-cts.exe ./tools/src/cmd/run-cts
cp out/Release/dawn_standalone_cts_runner package/
./tools/run run-cts zip-cts -o package/cts.zip
```

On the test machine, run it from the package's directory. Outside a Dawn
checkout, `run-cts` has no default runner or CTS, so name both:

```sh
./run-cts standalone -runner ./dawn_standalone_cts_runner -cts cts.zip \
    'webgpu:*'
```

For an Android device, build `run-cts` for the machine that the device is
connected to and the runner for Android, then add `-adb`.

## Running the runner directly

The runner needs neither `run-cts`, Go nor Node.js once the CTS JavaScript is
built.

To build the CTS JavaScript yourself, with `npm` and `npx` on your `PATH` as
[above](#running-with-run-cts):

```sh
cd third_party/webgpu-cts && npm ci && npx grunt node && cd ../..
```

Then run from the Dawn root, which finds the CTS in `third_party/webgpu-cts`, or
pass `--cts-dir <dir>` or `--cts-zip <zip>`:

```sh
out/Release/dawn_standalone_cts_runner -q 'webgpu:api,operation,queue,*'
out/Release/dawn_standalone_cts_runner --cts-zip cts.zip \
    -q 'webgpu:api,operation,queue,*'
```

`-q` may be repeated. `--query-file <file>` reads queries or test cases, one per
line, from a file, or from stdin when the file is `-`, and `--list` prints the
matching test cases instead of running them. `--help` lists the other flags, and
`--dawn-flag <key>=<value>` sets a [Dawn flag](#dawn-flags).

### Making the zip without Go

The zip holds `out-node/` without its `.map` files, and
`src/common/runtime/cmdline.ts`. On Linux or macOS, from the Dawn root:

```sh
rm -f cts.zip && (cd third_party/webgpu-cts &&
    zip -q -r ../../cts.zip out-node src/common/runtime/cmdline.ts -x '*.map')
```

On Windows, from `third_party\webgpu-cts` in PowerShell or a Command Prompt:

```powershell
tar.exe -a -c -f ..\..\cts.zip --exclude="*.map" out-node src/common/runtime/cmdline.ts
```

### On Android

Copy the runner and the zip to the device and run it through `adb shell`:

```sh
DEV=/data/local/tmp/cts
adb shell "mkdir -p $DEV"
adb push out/Android/dawn_standalone_cts_runner cts.zip $DEV/
adb shell "chmod 755 $DEV/dawn_standalone_cts_runner"
adb shell "$DEV/dawn_standalone_cts_runner --cts-zip $DEV/cts.zip \
    -q 'webgpu:api,operation,queue,*'"
```

## Dawn flags

`run-cts standalone -flag` and the runner's `--dawn-flag` take a `key=value` for
the Dawn provider, and may be repeated. The keys are:

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
