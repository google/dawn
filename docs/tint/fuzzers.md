# Tint Fuzzers

Tint currently has two fuzzer target executables: [`tint_wgsl_fuzzer`](#tint_wgsl_fuzzer) which takes WGSL source code as input and [`tint_ir_fuzzer`](#tint_ir_fuzzer) which takes a protobuf binary file as input.

Both fuzzers are implemented using [libFuzzer](https://llvm.org/docs/LibFuzzer.html), and are automatically and continuously run by Chromium's ClusterFuzz infrastructure. [Fuzzer targets are automatically found via `gn refs`](https://chromium.googlesource.com/chromium/src/+/HEAD/testing/libfuzzer/libFuzzer_integration.md). See [`tint.gni`](../../src/tint/tint.gni) for the core fuzzer target rules.

Tint's fuzzers are implemented as functions registered with the macros:

- [`TINT_WGSL_PROGRAM_FUZZER()`](#registering-a-new-tintprogram-fuzzer) registers a fuzzer function that is handed a `tint::Program`.
- [`TINT_IR_MODULE_FUZZER()`](#registering-a-new-tintcoreirmodule-fuzzer) registers a fuzzer function that is handed a `tint::core::ir::Module`.

## Building

The fuzzer targets can be build with either CMake or GN:

- CMake: Define `TINT_BUILD_FUZZERS=1` (pass `-DTINT_BUILD_FUZZERS=1` to `CMake`, or set in `CMakeCache.txt`)
- GN: Define `use_libfuzzer = true` in `args.gn`.

## Local fuzzing

The [`tint_wgsl_fuzzer`](#tint_wgsl_fuzzer) and [`tint_ir_fuzzer`](#tint_ir_fuzzer) executables accept the [standard `libFuzzer` command line arguments](https://llvm.org/docs/LibFuzzer.html#options) with extended command line arguments described below.

There's also a helper tool to run the fuzzers locally:

- To run the local fuzzers across the full number of CPU threads available on the system, seeded with the corpus in [`test/tint`](../../test/tint), run:

  `tools/run fuzz`

- To check that all the test files in [`test/tint`](../../test/tint) pass the fuzzers without error and then exit, run:

  `tools/run fuzz --check`

  Note: This is run by Dawn's CQ presubmit to check that fuzzers aren't accidentally broken.

## Registering a new `tint::Program` fuzzer

1. Create a new source file with a `_fuzz.cc` suffix.
2. `#include "src/tint/cmd/fuzz/wgsl/fuzz.h"`
3. Define a function in a (possibly nested) anonymous namespace with one of the signatures:

   - `void MyFuzzer(const tint::Program& program /*, ...additional fuzzed parameters... */) {`
   - `void MyFuzzer(const tint::Program& program, const tint::fuzz::wgsl::Context& context /*, ...additional fuzzed parameters... */) {`

   The optional `context` parameter holds information about the `Program` and the environment used to run the fuzzers. \
    [Note: Any number of additional fuzzer-populated parameters can be appended to the function signature.](#additional-fuzzer-data)

4. Implement your fuzzer function, using `TINT_ICE()` to catch invalid state. Return early if the fuzzer cannot handle the input.
5. At the bottom of the file, in the global namespace, register the fuzzer with: `TINT_WGSL_PROGRAM_FUZZER(MyFuzzer);`
6. Use `tools/run gen build` to generate the build files for this new fuzzer.

Example:

```c++
#include "src/tint/cmd/fuzz/wgsl/fuzz.h"

namespace tint::my_namespace {
namespace {

bool CanRun(const tint::Program& program) {
    if (program.AST().HasOverrides()) {
        return false;  // Overrides are not supported.
    }
    return true;
}

void MyWGSLFuzzer(const tint::Program& program, bool a_fuzzer_provided_value) {
    if (!CanRun(program)) {
        return;
    }

    // Check something with program.
}

}  // namespace
}  // namespace tint::my_namespace

TINT_WGSL_PROGRAM_FUZZER(tint::my_namespace::MyWGSLFuzzer);

```

## Registering a new `tint::core::ir::Module` fuzzer

1. Create a new source file with a `_fuzz.cc` suffix.
2. `#include "src/tint/cmd/fuzz/ir/fuzz.h"`
3. Define a function in a (possibly nested) anonymous namespace with the signature:

   - `void MyFuzzer(core::ir::Module& module /*, ...additional fuzzed parameters... */) {`

   [Note: Any number of additional fuzzer-populated parameters can be appended to the function signature.](#additional-fuzzer-data)

4. Implement your fuzzer function, using `TINT_ICE()` to catch invalid state. Return early if the fuzzer cannot handle the input.
5. At the bottom of the file, in the global namespace, register the fuzzer with: `TINT_IR_MODULE_FUZZER(MyFuzzer);`
6. Use `tools/run gen build` to generate the build files for this new fuzzer.

Example:

```c++
#include "src/tint/cmd/fuzz/ir/fuzz.h"

namespace tint::my_namespace {
namespace {

void MyIRFuzzer(core::ir::Module& module) {
    // Do something interesting with module.
}

}  // namespace
}  // namespace tint::my_namespace

TINT_IR_MODULE_FUZZER(tint::my_namespace::MyIRFuzzer);

```

## Additional fuzzer data

WGSL and IR fuzzer functions can also declare any number of additional parameters, which will be populated with fuzzer provided data. These additional parameters must come at the end of the signatures described above, and can be of the following types:

- Any integer, float or bool type.
- Any structure reflected with `TINT_REFLECT`. Note: It's recommended to use a `const` reference, for these to avoid pass-by-value overheads.
- Any enum reflected with `TINT_REFLECT_ENUM_RANGE`.

## Executable targets

Tint has two fuzzer executable targets:

### `tint_wgsl_fuzzer`

`tint_wgsl_fuzzer` [accepts WGSL textual input](https://llvm.org/docs/LibFuzzer.html#options) and parses line comments (`//`) as a base-64 binary encoded data stream for the [additional fuzzer parameters](additional-fuzzer-data).

The entry point for the fuzzer lives at [`src/tint/cmd/fuzz/wgsl/main_fuzz.cc`](../../src/tint/cmd/fuzz/wgsl/main_fuzz.cc).

#### Command line flags

- `--help`: lists the command line arguments.
- `--filter`: only runs the fuzzer functions that contain the given string in its name.
- `--concurrent`: each of the fuzzer functions will be run on a separate, concurrent thread. This potentially offers performance improvements, and also tests for concurrent execution.
- `--verbose` : prints verbose information about what the fuzzer is doing.

#### Behavior

The `tint_wgsl_fuzzer` will do the following:

- Base-64 decode the line comments data from the WGSL source, used to populate the [additional fuzzer parameters](additional-fuzzer-data).
- Parse and resolve the WGSL input, and will early-return if there are any parser errors.
- Invoke each of the fuzzer functions registered with a call to `TINT_WGSL_PROGRAM_FUZZER()`
- Automatically convert the `Program` to an IR module and run the function for each function registered with `TINT_IR_MODULE_FUZZER()`. Note: The `Program` is converted to an IR module for each registered IR fuzzer as the module is mutable.

### `tint_ir_fuzzer`

TODO: Document when landed.
