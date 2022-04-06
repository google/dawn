# Tint end-to-end tests

This repo contains a large number of end-to-end tests at `<tint>/test`.

## Test files

Test input files have either the `.wgsl`, `.spv` or `.spvasm` file extension.

Each test input file is tested against each of the Tint backends. There are `<number-of-input-files>` &times; `<number-of-tint-backends>` tests that are performed on an unfiltered end-to-end test run.

Each backend test can have an **expectation file**. This expectation file sits next to the input file, with a `<input-file>.expected.<format>` extension. For example the test `test/foo.wgsl` would have the HLSL expectation file `test/foo.wgsl.expected.hlsl`.

An expectation file contains the expected output of Tint, when passed the input file for the given backend.

If the first line of the expectation file starts `SKIP`, then the test will be skipped instead of failing the end-to-end test run. It is good practice to include after the `SKIP` a reason for why the test is being skipped, along with any additional details, such as compiler error messages.

## Running

To run the end-to-end tests use the `<tint>/test/test-all.sh` script, passing the path to the tint executable as the first command line argument.

You can pass `--help` to see the full list of command line flags.\
The most commonly used flags are:

| flag                 | description |
|----------------------|-------------|
|`--filter`            | Filters the testing to subset of the tests. The filter argument is a glob pattern that can include `*` for any substring of a file or directory, and `**` for any number of directories.<br>Example: `--filter 'expressions/**/i32.wgsl'` will test all the `i32.wgsl` expression tests.
|`--format`            | Filters the tests to the particular backend.<br>Example: `--format hlsl` will just test the HLSL backend.
|`--generate-expected` | Generate expectation files for the tests that previously had no expectation file, or were marked as `SKIP` but now pass.
|`--generate-skip`     | Generate `SKIP` expectation files for tests that are not currently passing.

## Authoring guidelines

Each test should be as small as possible, and focused on the particular feature being tested.

Use sub-directories whenever possible to group similar tests, and try to keep the pattern of directories as consistent as possible between different tests. This helps filter tests using the `--filter` glob patterns.
