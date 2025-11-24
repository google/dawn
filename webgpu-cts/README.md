# Running the WebGPU CTS Locally with Chrome

Running the WebGPU CTS locally with Chrome requires a Chromium checkout.
Follow [these instructions](https://www.chromium.org/developers/how-tos/get-the-code/)
for checking out and building Chrome.

**Build the `telemetry_gpu_integration_test` targets.**

At the root of a Chromium checkout, run:
`./content/test/gpu/run_gpu_integration_test.py webgpu_cts --browser=exact --browser-executable=path/to/your/chrome-executable`

If you don't want to build Chrome, you can still run the CTS, by passing the path to an existing Chrome executable to the `--browser-executable` argument. However, if you would like to use all harness functionality (symbolizing stack dumps, etc.). You will still need to build the `telemetry_gpu_integration_test` target.

You will probably want these command line arguments:

- `--passthrough --show-stdout`: Show browser output. See also `--browser-logging-verbosity`.
- `--test-filter='webgpu:examples:*'`: Filter tests.
- `--jobs=1`: Run just one browser instance, best for debugging. Omit or set higher if you need to run more than a few tests.
- `--write-full-results-to=results.json`
- Also note artifacts like crash logs are written to `artifacts/`

These GPU-specific arguments are often needed:

- `--force_low_power_gpu` and `--force_high_performance_gpu`: Set the GPU used by the whole browser,
  and thus the (default) adapter for WebGPU. (On dual-GPU Macs, only, Chrome can use multiple GPUs.
  [`--use-webgpu-power-preference`][use-webgpu-power-preference] can be used to control which one WebGPU chooses.)
- `--enable-dawn-backend-validation`: Enable Dawn's backend validation.
- `--use-webgpu-adapter=[default,swiftshader,compat]`: Forwarded to the browser to select a particular WebGPU adapter.

[use-webgpu-power-preference]: https://source.chromium.org/chromium/chromium/src/+/main:gpu/command_buffer/service/service_utils.cc;l=114-121;drc=73bcf25e4b0c06b1a86422a2fc30023c7b850f33

Other useful command-line arguments:

- `--help`: See more options and argument documentation.
- `-l`: List all tests that would be run.
- `--extra-browser-args`: Pass extra args to the browser executable.
- `--stable-jobs`: Assign tests to each job in a stable order. Used on the bots
  for consistency and ease of reproduction. If you are reproducing
  order-dependent issues that appear on bots, you'll need this and `--jobs=N`.
- `--no-browser-restart-on-failure`: Faster runs, but some types of failure
  (e.g. real device loss) will cause many subsequent tests to fail to start.
- `--skip-post-test-cleanup-and-debug-info`: Faster runs, "at the cost of
  providing less actionable data when a test does fail."

## Running the CTS locally on Android

If you want to run the full CTS on Android with expectations locally, some additional setup is required.

First, as explained in the [GPU Testing doc](https://source.chromium.org/chromium/chromium/src/+/main:docs/gpu/gpu_testing.md) you need to build the `telemetry_gpu_integration_test_android_chrome` target. This target _must_ be built in either the `out/Release` or `out/Debug` directories, as those are the only two that can be targeted by the test runner when executing tests on Android.

When executing the tests, use `--browser=android-chromium` (which will look in `out/Release` by default) and limit the tests to one job at a time with `--jobs=1`.

An example of a known-working command line is:

```sh
./content/test/gpu/run_gpu_integration_test.py webgpu_cts --passthrough --show-stdout --stable-jobs --jobs=1 --extra-browser-args="--enable-logging=stderr --js-flags=--expose-gc" --write-full-results-to=results.json --browser=android-chromium
```

For Compat tests, run `webgpu_compat_cts` instead.

Be aware that running the tests locally on Android is *SLOW*. Expect it to take 4 hrs+.
(To run faster, look at `--no-browser-restart-on-failure --skip-post-test-cleanup-and-debug-info` mentioned above.)

### Running on WebView

Similar to above, except:

To test the default WebView configuration, follow the steps above, except:

1. Build `telemetry_gpu_integration_test_android_webview`.
1. Use the `run_gpu_integration_test` command above, except with the browser set to
    `--browser=android-webview-instrumentation` instead.

If you need an interactive WebView shell, build `system_webview_shell_apk` and install it
using `bin/system_webview_shell_apk` or from `apks/SystemWebViewShell.apk`.

For [mixed-bitness WebView](https://chromium.googlesource.com/chromium/src/+/HEAD/android_webview/docs/architecture.md#bitness):

1. Use a device that supports both 32-bit and 64-bit binaries, like Pixel < 7.
1. Set GN args `target_cpu = "arm64"` and `enable_android_secondary_abi = true`.
1. Build `telemetry_gpu_integration_test_android_webview`.
1. To test a 64-bit host/browser/GPU process with a 32-bit renderer process:

    1. Build the additional `system_webview_shell_apk` and `trichrome_webview_32_64_bundle` targets.
    1.
        ```sh
        adb install --abi arm64-v8a out/${BUILD_TYPE}/apks/SystemWebViewShell.apk
        out/${BUILD_TYPE}/bin/trichrome_webview_32_64_bundle install
        out/${BUILD_TYPE}/bin/trichrome_webview_32_64_bundle set-webview-provider
        ```

    To test a 32-bit host/browser/GPU process with a 64-bit renderer process:

    1. Build the additional `system_webview_shell_apk` and `trichrome_webview_64_32_bundle` targets.
    1.
        ```sh
        adb install --abi armeabi-v7a out/${BUILD_TYPE}/apks/SystemWebViewShell.apk
        out/${BUILD_TYPE}/bin/trichrome_webview_64_32_bundle install
        out/${BUILD_TYPE}/bin/trichrome_webview_64_32_bundle set-webview-provider
        ```
    1. Be sure to uninstall the WebView Shell app before running other tests if you want to go back to a 64-bit host.

1. Use the `run_gpu_integration_test` command above, except with the browser set to
    `--browser=android-webview --assume-browser-already-installed` instead.

### Running without root

Typically you want to run the CTS on a device which has root, which generally means flashing a userdebug image onto the device. If this isn't an option, you can try running with the `--compatibility-mode=dont-require-rooted-device` flag as described on [this page](https://chromium.googlesource.com/catapult/+/HEAD/telemetry/docs/run_benchmarks_locally.md), though this is not a supported configuration and you may run into errors.

This mode has been observed to fail if another version of Chrome besides the `chrome_public_apk` target is currently running on the device, so it's suggested to manually close all Chrome variants before starting the test.

### Android Proxy errors

When running the tests on Android devices with the above commands, some devices have been observed to start displaying an `ERR_PROXY_CONNECTION_FAILED` error when attempting to browse with Chrome/Chromium. This is the result of command line proxy settings used by the test runner accidentally not getting cleaned up, likely because the script was terminated early. Should it happen to you the command line used by Chrome can be cleared by running the following command from the root of a Chromium checkout:

```sh
build/android/adb_chrome_public_command_line ""  # /data/local/tmp/chrome-command-line
```

Similarly for WebView and WebViewInstrumentation:

```sh
build/android/adb_system_webview_command_line ""            # /data/local/tmp/webview-command-line
out/Release/bin/webview_instrumentation_apk argv --args ""  # /data/local/tmp/android-webview-command-line
```

# Running a local CTS build on Swarming

Often, it's useful to test changes on Chrome's infrastructure if it's difficult to reproduce a bug locally. To do that, we can package our local build as an "isolate" and upload it to Swarming to run there. This is often much faster than uploading your CL to Gerrit and triggering tryjobs.

Note that since you're doing a local build, you need to be on the same type of machine as the job you'd like to trigger in swarming. To run a job on a Windows bot, you need to build the isolate on Windows.

1. Build the isolate

   `vpython3 tools/mb/mb.py isolate out/Release telemetry_gpu_integration_test`
2. Upload the isolate

   `./tools/luci-go/isolate archive -cas-instance chromium-swarm -i out/Release/telemetry_gpu_integration_test.isolate`

   This will output a hash like:
   95199eb624d8ddb6ffdfe7a2fc41bc08573aebe3d17363a119cb1e9ca45761ae/734

   Save this hash for use in the next command.
3. Trigger the swarming job.

   The command structure is as follows:

   `./tools/luci-go/swarming trigger -S https://chromium-swarm.appspot.com <dimensions...> -digest <YOUR_ISOLATE_HASH> -- <command> ...`

   Say you want to trigger a job on a Linux Intel bot. It's easiest to check an existing task to see the right args you would use.
   For example: https://chromium-swarm.appspot.com/task?id=5d552b8def31ab11.

   In the table on the left hand side, you can see the bot's **Dimensions**.
   In the **Raw Output** on the right or below the table, you can see the commands run on this bot. Copying those, you would use:
   ```
   ./tools/luci-go/swarming trigger -S https://chromium-swarm.appspot.com -d "pool=chromium.tests.gpu" -d "cpu=x86-64" -d "gpu=8086:9bc5-20.0.8" -d "os=Ubuntu-18.04.6" -digest 95199eb624d8ddb6ffdfe7a2fc41bc08573aebe3d17363a119cb1e9ca45761ae/734 -- vpython3 testing/test_env.py testing/scripts/run_gpu_integration_test_as_googletest.py content/test/gpu/run_gpu_integration_test.py --isolated-script-test-output=${ISOLATED_OUTDIR}/output.json webgpu_cts --browser=release --passthrough -v --show-stdout --extra-browser-args="--enable-logging=stderr --js-flags=--expose-gc --force_high_performance_gpu --enable-features=Vulkan" --total-shards=14 --shard-index=0 --jobs=4 --stable-jobs
   ```

   The command will output a link to the Swarming task for you to see the results.

# Browsing failure expectations

Dawn is tested against the CTS across various platforms and GPUs.
The failure results are checked into this directory as failure expectations files:

* [expectations.txt](expectations.txt): Core Webgpu functionality
* [compat-expectations.txt](compat-expectations.txt): WebGPU "compat" functionality

Use [failure_browser/index.html](failure_browser/index.html) to review the data
interactively.
