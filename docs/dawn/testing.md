# Testing Dawn

(TODO)

## Dawn Perf Tests

For benchmarking with `dawn_perf_tests`, it's best to build inside a Chromium checkout using the following GN args:
```
is_official_build = true  # Enables highest optimization level, using LTO on some platforms
use_dawn = true           # Required to build Dawn
use_cfi_icall=false       # Required because Dawn dynamically loads function pointers, and we don't sanitize them yet.
```

A Chromium checkout is required for the highest optimization flags. It is possible to build and run `dawn_perf_tests` from a standalone Dawn checkout as well, only using GN arg `is_debug=false`. For more information on building, please see [building.md](../building.md).

### Terminology

 - Iteration: The unit of work being measured. It could be a frame, a draw call, a data upload, a computation, etc. `dawn_perf_tests` metrics are reported as time per iteration.
 - Step: A group of Iterations run together. The number of `iterationsPerStep` is provided to the constructor of `DawnPerfTestBase`.
 - Trial: A group of Steps run consecutively. `kNumTrials` are run for each test. A Step in a Trial is run repetitively for approximately `kCalibrationRunTimeSeconds`. Metrics are accumlated per-trial and reported as the total time divided by `numSteps * iterationsPerStep`. `maxStepsInFlight` is passed to the `DawnPerfTestsBase` constructor to limit the number of Steps pipelined.

(See [`//src/dawn/tests/perf_tests/DawnPerfTest.h`](https://cs.chromium.org/chromium/src/third_party/dawn/src/dawn/tests/perf_tests/DawnPerfTest.h) for the values of the constants).

### Metrics

`dawn_perf_tests` measures the following metrics:
 - `wall_time`: The time per iteration, including time waiting for the GPU between Steps in a Trial.
 - `cpu_time`: The time per iteration, not including time waiting for the GPU between Steps in a Trial.
 - `validation_time`: The time for CommandBuffer / RenderBundle validation.
 - `recording_time`: The time to convert Dawn commands to native commands.

Metrics are reported according to the format specified at
[[chromium]//build/recipes/performance_log_processor.py](https://cs.chromium.org/chromium/build/recipes/performance_log_processor.py)

### Dumping Trace Files

The test harness supports a `--trace-file=path/to/trace.json` argument where Dawn trace events can be dumped. The traces can be viewed in Chrome's `about://tracing` viewer.

### Test Runner

[`//scripts/perf_test_runner.py`](https://cs.chromium.org/chromium/src/third_party/dawn/scripts/perf_test_runner.py) may be run to continuously run a test and report mean times and variances.

Currently the script looks in the `out/Release` build directory and measures the `wall_time` metric (hardcoded into the script). These should eventually become arguments.

Example usage:

```
scripts/perf_test_runner.py DrawCallPerf.Run/Vulkan__e_skip_validation
```

### Tests

**BufferUploadPerf**

Tests repetitively uploading data to the GPU using either `WriteBuffer` or `CreateBuffer` with `mappedAtCreation = true`.

**DrawCallPerf**

DrawCallPerf tests drawing a simple triangle with many ways of encoding commands,
binding, and uploading data to the GPU. The rationale for this is the following:
  - Static/Multiple/Dynamic vertex buffers: Tests switching buffer bindings. This has
    a state tracking cost as well as a GPU driver cost.
  - Static/Multiple/Dynamic bind groups: Same rationale as vertex buffers
  - Static/Dynamic pipelines: In addition to a change to GPU state, changing the pipeline
    layout incurs additional state tracking costs in Dawn.
  - With/Without render bundles: All of the above can have lower validation costs if
    precomputed in a render bundle.
  - Static/Dynamic data: Updating data for each draw is a common use case. It also tests
    the efficiency of resource transitions.

## Testing with the CTS

There are various ways to test dawn against the [WebGPU CTS](https://github.com/gpuweb/cts).

### With dawn.node
See [src/dawn/node/README.md](../../src/dawn/node/README.md).

### With a Chromium build manually
Go to [https://gpuweb.github.io/cts/standalone/](https://gpuweb.github.io/cts/standalone)
or clone [the CTS repo](https://github.com/gpuweb/cts) and serve it locally

```sh
git clone https://github.com/gpuweb/cts.git
cd cts
npm ci
npm start
```

Then open `http://localhost:8080/standalone`.

### On the CQ
You can run a specific version of the CTS with top of tree Dawn in Chromium.
First, upload the version you want to test to your own github fork of the CTS.
Note the commit-hash. Then run:

```sh
./tools/run cts roll --max-attempts 0 --repo <repo-url> --revision <commit-hash>
```

Example:

```sh
./tools/run cts roll --max-attempts 0 --repo https://github.com/greggman/cts.git --revision 1b3173b36a1084917e0ff29e5f40c6290b6f8adc
```

The roll script take a couple of minutes as it downloads that version of the CTS and
creates a roll patch. Once the roll script says it's waiting on the bots to finish
you can kill (Ctrl-C) the script. The CQ will continue to run the tests.

#### Filtering - running less than the full CTS
You can also specify a CTS query with `--test-query <query>`, for example, test all the
WGSL builtins `... --test-query "webgpu:shader,execution,expression,call,builtin,*"`.
You can also filter tests with a glob `--test-filter <glob>`, for example, only run tests
with `BindGroup` in the test name `... --test-filter "*BindGroup*"`.

See  `tools/src/cmd/cts/roll/roll.go` for other options.

#### Testing with a different version of Dawn

The roll is just a normal CL on gerrit so, after creating a roll CL, follow the instructions
on gerrit to download that CL. Apply any dawn changes. Then re-upload with
`git cl upload --force --bypass-hooks` and start a dry run on gerrit.
