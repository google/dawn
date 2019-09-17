# Dawn Perf Tests

## Tests

**BufferUploadPerf**

Tests repetitively uploading data to the GPU using either `SetSubData` or `CreateBufferMapped`.

## Test Harness
The test harness provides a `DawnPerfTestBase` which Derived tests should inherit from.
The harness calls `Step()` of a Derived class to measure its execution
time. First, a calibration step is run which determines the number of times
to call `Step()` to last approximately `kCalibrationRunTimeSeconds`. Then,
`Step()` is called for the computed number of times, or until
`kMaximumRunTimeSeconds` is exceeded. `kNumTrials` are performed and the
results and averages per iteration\* are printed.
(See `DawnPerfTest.h` for the values of the constants).

The results are printed according to the format specified at
[[chromium]//build/scripts/slave/performance_log_processor.py](https://cs.chromium.org/chromium/build/scripts/slave/performance_log_processor.py)

\*The number of iterations a test performs should be passed to the
constructor of `DawnPerfTestBase`. The reported times are the total time
divided by `numSteps * iterationsPerStep`.
