# Running the WebGPU CTS Locally with Chrome

Running the WebGPU CTS locally with Chrome requires a Chromium checkout.

Follow [these instructions](https://www.chromium.org/developers/how-tos/get-the-code/) for checking out
and building Chrome. You'll also need to build the `telemetry_gpu_integration_test` target.

At the root of a Chromium checkout, run:
`./content/test/gpu/run_gpu_integration_test.py webgpu_cts --browser=exact --browser-executable=path/to/your/chrome-executable`

If you don't want to build Chrome, you can still run the CTS, by passing the path to an existing Chrome executable to the `--browser-executable` argument. However, if you would like to use all harness functionality (symbolizing stack dumps, etc.). You will still need to build the `telemetry_gpu_integration_test` target.

Useful command-line arguments:
 - `--help`: See more options and argument documentation.
 - `-l`: List all tests that would be run.
 - `--test-filter`: Filter tests.
 - `--passthrough --show-stdout`: Show browser output. See also `--browser-logging-verbosity`.
 - `--extra-browser-args`: Pass extra args to the browser executable.
 - `--jobs=N`: Run with multiple parallel browser instances.
 - `--stable-jobs`: Assign tests to each job in a stable order. Used on the bots for consistency and ease of reproduction.
 - `--enable-dawn-backend-validation`: Enable Dawn's backend validation.
 - `--use-webgpu-adapter=[default,swiftshader,compat]`: Forwarded to the browser to select a particular WebGPU adapter.

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
