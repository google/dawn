# Tint Fuzzing Experiments

The Tint fuzzing experiment/benchmarking framework is designed to
automate the process of comparing changes to the Tint fuzzers. It
measures hardware performance, fuzzer coverage percentage rates, and
test case execution rates to come up with abstract performance scores
to compare versions of the fuzzers.

This framework is part of the general fuzz utility for Tint found in
`./tools/src/cmd/fuzz`.

This document explains the high-level architecture of the experiments
framework, the environment prerequisites, and a step-by-step
walkthrough of how to define, execute, and process an
experiment. Along with a reference section at the end for the
experiment configuration JSON format,

---

## Prerequisites

Running fuzzing experiments requires a local Dawn repository and a GN
build directory configured for fuzzing and coverage.

To support correct instrumentation, your GN build (`args.gn`) must
include at least the following flags:

```gn
use_libfuzzer = true
tint_build_wgsl_reader = true
use_clang_coverage = true
optimize_for_fuzzing = false  # Cannot be used with coverage enabled
```

Additional flags must be enabled depending on the fuzzers selected:
- **IR Fuzzers (`tint_ir_fuzzer`, `tint_ir_mesa_fuzzer` )**: Require
  `tint_build_ir_binary = true` and `tint_has_protobuf = true`.
- **Mesa Fuzzers (`tint_wgsl_mesa_fuzzer`, `tint_ir_mesa_fuzzer`)**:
  Require `tint_build_mesa = true` and
  `tint_build_fuzzer_vulkan_support = true`.

The tool will produce an error if the flags are not correctly
configured. The tool does not support using a pre-built version of the
Tint fuzzers, nor the CMake builds.

**Note:** For the most applicable results it is recommended that all
backends are turned on (`tint_build_hlsl_writer = true`,
`tint_build_msl_writer = true`, `tint_build_spv_writer = true`)

**Note:** Though the fuzzers can be built and run with sanitizers
turned on (`is_asan = true`, etc), this is not recommended, because
the sanitizers tend to dominate the runtime of the execution, so
produce substantially lower quality data

---

## Defining an Experiment

An experiment is represented by a dedicated directory containing:
1. An `experiment.json` configuration file.
2. A `corpora/` directory holding the seed corpora for the various
   fuzzing modes.

### Directory Structure

```directory
my_experiment/
├── experiment.json
└── corpora/
    ├── wgsl_seed/
    │   ├── shader1.wgsl
    │   └── shader2.wgsl
    └── empty/
```

### Writing `experiment.json`

The `experiment.json` defines variables like the commit hash to check
out, specific fuzzers to test, and duration of the experiments.

Here is an example setup for comparing fuzzer performance using both a
standard corpus and an empty directory as starting points:

```json
{
    "name": "example",
    "hash": "4c2395a860ab76b44f7256c25281dfd3a3680192",
    "fuzzers": ["tint_wgsl_fuzzer", "tint_ir_fuzzer"],
    "burnin_enabled": false,
    "normalization_duration": 10,
    "normalization_iterations": 2,
    "wgsl_normalization_corpus": "wgsl_seed",
    "ir_normalization_corpus": "empty",
    "wgsl_corpora": [
        { "name": "wgsl_full", "path": "wgsl_seed" },
        { "name": "wgsl_empty", "path": "empty" }
    ],
    "ir_corpora": [
        { "name": "ir_empty", "path": "empty" }
    ],
    "default_iterations": 5,
    "durations": [
        {"runs": 10000},
        {"runs": 50000},
        {"runs": 100000}
    ]
}
```

- `name` is just a user visible title for this experiment which will
  appear in reports/logging, but has no semantic meaning
- `hash` is the specific version of the Dawn repo that will be checked
  out to build the fuzzers. (The tool will be built and run from what
  ever hash the repo is at when you call it, not this version)
- `fuzzers` are the specific fuzzer binaries to test
- `burnin_enabled`, `normalization_duration`, and
  `normalization_iterations` are turned down or off in this example so
  that the setup phases run fast, but these should be removed or
  increased when generating statistically valid data
- `*_normalization_corpus` are the corpora to use when establishing
  baselines for normalizing performance numbers between
  machines/environments
- `*_corpora` are the various starting corpora to experiment using
- `default_iterations` specifies how many times to run each experiment
  if not explicitly overridden
- `durations` specifies the sets experiments to run, broadly you will
  want to specify a spread of values here to get a reasonable
  graph performance over time, since the framework doesn't
  sample these values during the experiment run

Full details on the various options are detailed below.

**Note:** The number of actual runs performed will be `# of fuzzers` X
`# of corpora` X `# of durations` X `# of iterations`

---

## Running an Experiment

To start the experiment on your system, execute the `fuzz` helper with
the `-experiment` flag:

```bash
/tools/run fuzz -experiment -build out/fuzzers my_experiment/
```

where `out/fuzzers` is a GN configured build with the correct values
set in `args.gn`

**Note:** `-experiment` does support the `-j` flag for running
concurrent operations, but defaults it to **1**. This is because
saturating the RAM or CPU in the execution environment (e.g. -j `max
num of cores`) can significantly impact the results. It is recommended
to experiment with a mini-version of an experiment.json to determine
an appropriate value to use, (try num of cores / 2 or / 4 as a
starting point)

### Execution Pipeline

The framework guides execution through the following phases:

1. **Validation & Sync**: The tool checks the active GN arguments in
   your build directory to ensure they match the requirements. It then
   checks out the requested commit hash (`hash` field) and runs
   `gclient sync` automatically. (The state of repo should be returned
   to the original state)
2. **Binary Preparation**: It builds the selected fuzzer binaries and
   copies them, alongside their dynamic library dependencies and
   required LLVM tools (`llvm-profdata`, `llvm-cov`), into
   `my_experiment/bin/`.
3. **Burn-in (Optional)**: If enabled, it executes multiple parallel
   workloads for 5 minutes (`burnin_duration` & `burnin_enabled`) to
   bring the physical host's CPU to a thermal steady state. This
   prevents throttling and normalization profiling skews during the
   experiment.
4. **Normalization**: It executes multiple normalization profiling
   runs for each fuzzer against its designated normalization corpus
   (controlled by `normalization_duration` and
   `normalization_iterations`). The resulting average execution rate
   (runs/sec) and standard error (SEM) are saved to
   `normalization_scores.json` and are used to compute "Normalized CPU
   Seconds" for each fuzzer. All individual profiling runs are
   archived in `normalization_iterations.csv`.
5. **Task Execution**: It calculates the Cartesian product of
   experiments (Fuzzer × Corpus × Duration × Iteration). These are run
   concurrently across available CPU cores (configurable via
   `-j`). Results are saved directly under
   `my_experiment/results/`. Each task directory stores a `state.json`
   file, captured logs, mutated corpus files, and `.profraw` coverage
   profiles.

---

## Processing and Analyzing Results

Once execution has completed, you can aggregate and process results
using the `-analyze` flag:

```bash
tools/run fuzz -analyze my_experiment/
```

### Analysis Pipeline

1. **Coverage Generation**: The analyzer locates the `.profraw` file
   for each completed iteration, merges them using `llvm-profdata`,
   and executes `coverage.py` to produce standard `.lcov` coverage
   logs.
2. **Component Mapping**: It parses the `.lcov` profiles and
   aggregates line coverage metrics separately for **Tint**
   (`src/tint/` and `src/utils/`), **Mesa** (`third_party/mesa/`), and
   **DXC** (`third_party/directx-headers/`).
3. **Statistics Computation**: It computes arithmetic means and
   standard errors for:
   - Coverage percentage
   - Normalized CPU seconds
   - Coverage rates (% of lines covered per CPU second, using
     quadrature error propagation)
4. **Report Export**:
   - `raw_iteration_data.csv`: A flat table of raw execution counts,
     actual runtimes, and coverage hits for every single iteration.
   - `calculated_statistics.csv`: A flat table of calculated
     statistics for every experiment category (Fuzzer × Corpus).
   - `experiment_report.md`: A human-readable Markdown summary report.

---

## Appendix: `experiment.json` Reference

The following sections define the full configuration schema for
`experiment.json`.

### Root Attributes

| Field                       | Type                   | Description                                                                                                                                  |
|:----------------------------|:-----------------------|:---------------------------------------------------------------------------------------------------------------------------------------------|
| `name`                      | `string`               | A human-readable identifier for the experiment, used in the logging/reports.                                                                 |
| `hash`                      | `string`               | The Git commit hash checkout from which the binaries should be compiled.                                                                     |
| `fuzzers`                   | `array of strings`     | Fuzzer target names to test(Supported values: `"tint_wgsl_fuzzer"`, `"tint_ir_fuzzer"`, `"tint_wgsl_mesa_fuzzer"`, `"tint_ir_mesa_fuzzer"`). |
| `timeout`                   | `integer` *optional*   | Timeout limit in seconds for a single fuzzer execution on a test case in libFuzzer (defaults to 5).                                          |
| `burnin_enabled`            | `boolean` *optional*   | If true, launches parallel workloads to warm up the machine before benching (defaults to true).                                              |
| `burnin_duration`           | `integer` *optional*   | Target duration in seconds for the initial thermal burn-in (defaults to 300).                                                                |
| `normalization_duration`    | `integer` *optional*   | Execution duration in seconds for each iteration of the normalization microbenchmarking phase (defaults to 60).                              |
| `normalization_iterations`  | `integer` *optional*   | Number of iterations to perform for normalization microbenchmarking (defaults to 5).                                                         |
| `wgsl_normalization_corpus` | `string`               | Directory path relative to the root `corpora/` to use when performing normalization on WGSL fuzzers.                                         |
| `ir_normalization_corpus`   | `string`               | Directory path relative to the root `corpora/` to use when performing normalization on IR fuzzers.                                           |
| `wgsl_corpora`              | `array of CorpusDef`   | Corpora definitions available to run with WGSL fuzzers.                                                                                      |
| `ir_corpora`                | `array of CorpusDef`   | Corpora definitions available to run with IR fuzzers.                                                                                        |
| `default_iterations`        | `integer`              | The default number of times to repeat every Fuzzer/Corpus/Duration combination if not otherwise specified.                                   |
| `durations`                 | `array of DurationDef` | The target run lengths defining the experiment stopping criteria.                                                                            |

### `CorpusDef` Format

| Field  | Type     | Description                                                              |
|:-------|:---------|:-------------------------------------------------------------------------|
| `name` | `string` | A human-readable identifier for the corpus, used in the logging/reports. |
| `path` | `string` | Directory path relative to the root `corpora/` directory.                |

### `DurationDef` Format

**Note**: Must specify exactly one and only one of `seconds` or
`runs`.

**Note**: Because of how libFuzzer operate `seconds` and `runs` are
approximate values. Fuzzing should run for at least this limit, but
will normally be a little over.

| Field        | Type                 | Description                                                                                                                                               |
|:-------------|:---------------------|:----------------------------------------------------------------------------------------------------------------------------------------------------------|
| `seconds`    | `integer` (optional) | Number of seconds to run for (maps to `-max_total_time`).                                                                                                 |
| `runs`       | `integer` (optional) | Number of inputs/test cases to run for(maps to `-runs`).                                                                                                  |
| `iterations` | `integer` (optional) | Override the experiment `default_iterations` specifically for this duration target. Useful for manually load balancing very long and very short durations |
