# LiteRT-LM Benchmarks

This document provides instructions for Dawn developers on how to configure, build, and run the LiteRT-LM benchmarks against Dawn, as well as how to inspect and interpret the benchmark output.

## Overview

LiteRT-LM is an on-device Large Language Model (LLM) inference runtime. In Dawn, it benchmarks real-world WebGPU compute shader execution and pipeline dispatch performance using the `libwebgpu_dawn` shared library.

### Benchmark Metrics

The benchmark reports several phases of execution. Not all reported numbers are affected by changes to Dawn:

#### Prefill Speed (`tokens/sec`)
Measures prompt evaluation throughput while processing input tokens into the KV-cache.

*WebGPU relevance:* Highly relevant. This phase is GPU compute-bound, exercising matrix multiplication and attention shaders as well as buffer memory bandwidth.

#### Decode Speed (`tokens/sec`)
Measures autoregressive generation throughput while predicting output tokens one by one.

*WebGPU relevance:* Highly relevant. This phase is memory-bandwidth and dispatch-latency bound, making it a direct measure of Dawn's command submission and dispatch overhead.

#### Time to First Token (TTFT)
Measures the wall-clock time from submitting the prompt to emitting the first generated token.

*WebGPU relevance:* Relevant. Dominated by the initial WebGPU prefill execution plus the first decode step.

#### Init Executor
Measures the time required to initialize the engine before inference begins.

*WebGPU relevance:* Relevant. Directly captures WebGPU device acquisition, pipeline layout creation, and shader compilation times.

#### Tokenizer and Metadata Operations
Includes `Init Tokenizer`, `Init LLM metadata`, and `TextToTokenIds`.

*WebGPU relevance:* Not relevant. These are host-side CPU operations (SentencePiece tokenization, protobuf decoding) that do not execute WebGPU code.

## 1. Fetch dependencies

LiteRT-LM sources, prebuilt accelerators, and benchmark model weights are managed conditionally in [`DEPS`](../../DEPS) to keep checkout sizes small for developers who do not need them.

To enable LiteRT-LM, add `"checkout_litert_lm": True` to `custom_vars` in your `.gclient` file:

```python
solutions = [
  {
    "name": ".",
    "url": "https://dawn.googlesource.com/dawn",
    "deps_file": "DEPS",
    "managed": False,
    "custom_vars": {
      "checkout_litert_lm": True,
    },
  },
]
```

After editing `.gclient`, sync dependencies:

```sh
gclient sync
```

This fetches Bazel (`tools/bazelisk/`), the LiteRT-LM source tree (`third_party/litert-lm/src/`), and the CIPD package containing `model.litertlm` and precompiled platform accelerators (`third_party/litert-lm/data/`).

## 2. Build

### Generate Build Directory

Use the [`tools/setup-build`](../../tools/setup-build) script to configure a release build:

```sh
./tools/setup-build gn release
```

This generates a GN build configuration in `out/gn-release` (and updates the `out/active` symlink to point to it) with optimizations enabled and debugging/sanitizer overhead disabled.

### Build the Benchmark Target

Build the benchmark executable using `autoninja`:

```sh
autoninja -C out/active litert_lm
```

### How the Build Works

1. GN first builds Dawn's monolithic shared library:
   - Linux: `out/active/libwebgpu_dawn.so`
   - macOS: `out/active/libwebgpu_dawn.dylib`
   - Windows: `out/active/libwebgpu_dawn.dll`
2. GN executes [`third_party/litert-lm/build_litert_lm.py`](../../third_party/litert-lm/build_litert_lm.py), which:
   - Configures Bazelisk with the hermetic Clang/LLVM toolchain from Dawn's `third_party/llvm-build`.
   - Links against the newly compiled local `libwebgpu_dawn` and platform prebuilts.
   - Builds the Bazel target `//runtime/engine:litert_lm_advanced_main`.
   - Copies the resulting `litert_lm_advanced_main` binary and required shared libraries into `out/active/`.

## 3. Run

You can run the benchmark binary directly from the root of your Dawn checkout.

### Linux

```sh
LD_LIBRARY_PATH=out/active ./out/active/litert_lm_advanced_main \
  --benchmark \
  --backend=gpu \
  --model_path=third_party/litert-lm/data/model.litertlm
```

### macOS

```sh
DYLD_LIBRARY_PATH=out/active ./out/active/litert_lm_advanced_main \
  --benchmark \
  --backend=gpu \
  --model_path=third_party/litert-lm/data/model.litertlm
```

### Windows

TODO: Not yet supported.

## Troubleshooting

### LiteRT-LM dependencies not found
Ensure `"checkout_litert_lm": True` is present under `custom_vars` in your `.gclient` file, then run `gclient sync`.

### GPU accelerator could not be loaded and registered
The prebuilt `libLiteRtWebGpuAccelerator` cannot find `libwebgpu_dawn`. Ensure that `LD_LIBRARY_PATH=out/active` (Linux) or `DYLD_LIBRARY_PATH=out/active` (macOS) is set when running the command.

### Symbol lookup errors or sanitizer crashes
Prebuilt LiteRT binaries are not built with sanitizers. Avoid ASan/UBSan configurations when benchmarking LiteRT-LM and use a clean `is_debug = false` release build.
