# Generating and viewing Tint code-coverage

Requirements:

* Host running Linux or macOS
* [vpython3](https://chromium.googlesource.com/chromium/tools/depot_tools/+/HEAD/vpython3)
  (usually provided by `depot_tools`)
* Clang toolchain

## Building Tint with coverage generation enabled

To generate coverage, you must build with Clang source-based coverage
instrumentation enabled.

### Using GN (Recommended)

Add the following flags to your `args.gn`:

```gn
use_clang_coverage = true
is_component_build = false
is_debug = false
dcheck_always_on = true
```

If you are building a fuzzer target, also include:

```gn
optimize_for_fuzzing = false
use_clang_modules = false
use_libfuzzer = true
```

### Using CMake

Add the additional `-DDAWN_EMIT_COVERAGE=1` CMake flag.

To use the `coverage.py` script's automated building features, you
must use the Ninja generator:

```bash
cmake -G Ninja -DDAWN_EMIT_COVERAGE=1 ..
```

If you use a different generator (like Make), you must build your
targets manually and use the `--no-compile` flag with the script.

## Generating Coverage Reports

Use the `tools/code_coverage/coverage.py` script to run your targets
and generate reports. (This is the Chromium maintained script that
replaces the previous Tint only script,
`tools/tint-generate-coverage`).

### 1. Generate LCOV for VSCode

To use the [VSCode Coverage
Gutters](https://marketplace.visualstudio.com/items?itemName=ryanluker.vscode-coverage-gutters)
extension, generate an LCOV tracefile:

```bash
vpython3 tools/code_coverage/coverage.py tint_unittests \
    -b out/gn-coverage \
    -o coverage_report \
    --format lcov \
    -c 'out/gn-coverage/tint_unittests'
```


The resulting file will be located at
`coverage_report/linux/coverage.lcov`.

### 2. Generate an HTML Report

To generate a browsable HTML report with line-by-line coverage:

```bash
vpython3 tools/code_coverage/coverage.py tint_unittests \
    --no-component-view \
    -b out/gn-coverage \
    -o coverage_report \
    --format html \
    -c 'out/gn-coverage/tint_unittests'
```

Open `coverage_report/linux/index.html` in your browser.

### 3. Advanced: Fuzzers with Dynamic Objects (Mesa/Vulkan)

If you are running a fuzzer that dynamically loads a Vulkan driver
(like Mesa's `lvp`), you need to explicitly include the driver object:

```bash
vpython3 tools/code_coverage/coverage.py tint_ir_fuzzer \
    --no-component-view \
    -b out/gn-coverage \
    -o coverage_report \
    --format html \
    -a out/gn-coverage/libvulkan_lvp.so \
    -i '\/usr\/.*' \
    -c 'out/gn-coverage/tint_ir_fuzzer -max_total_time=30'
```

*   `-a`: Adds the dynamically loaded driver as object source to the
    coverage calculation.
*   `-i '\/usr\/.*` is used to not include system files that are
    outside the source tree.

## Troubleshooting

### 403 error when generating HTML reports
If you see something like this:
```
[2026-04-13 13:20:20,757 INFO] Generating code coverage report in html (this can take a while depending on size of target!).
Traceback (most recent call last):
  File "/home/<user>/workspace/dawn/tools/code_coverage/coverage.py", line 1271, in <module>
    sys.exit(Main())
             ^^^^^^
  File "/home/<user>/workspace/dawn/tools/code_coverage/coverage.py", line 1266, in Main
    _GenerateCoverageReport(args, binary_paths, profdata_file_path,
  File "/home/<user>/workspace/dawn/tools/code_coverage/coverage.py", line 968, in _GenerateCoverageReport
    component_mappings = json.load(urlopen(COMPONENT_MAPPING_URL))
                                   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/<user>/.cache/vpython-root.119166/store/cpython+l5fnajrvijf7cvdkjqmbicg3i8/contents/lib/python3.11/urllib/request.py", line 216, in urlopen
    return opener.open(url, data, timeout)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/<user>/.cache/vpython-root.119166/store/cpython+l5fnajrvijf7cvdkjqmbicg3i8/contents/lib/python3.11/urllib/request.py", line 525, in open
    response = meth(req, response)
               ^^^^^^^^^^^^^^^^^^^
  File "/home/<user>/.cache/vpython-root.119166/store/cpython+l5fnajrvijf7cvdkjqmbicg3i8/contents/lib/python3.11/urllib/request.py", line 634, in http_response
    response = self.parent.error(
               ^^^^^^^^^^^^^^^^^^
  File "/home/<user>/.cache/vpython-root.119166/store/cpython+l5fnajrvijf7cvdkjqmbicg3i8/contents/lib/python3.11/urllib/request.py", line 563, in error
    return self._call_chain(*args)
           ^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/<user>/.cache/vpython-root.119166/store/cpython+l5fnajrvijf7cvdkjqmbicg3i8/contents/lib/python3.11/urllib/request.py", line 496, in _call_chain
    result = func(*args)
             ^^^^^^^^^^^
  File "/home/<user>/.cache/vpython-root.119166/store/cpython+l5fnajrvijf7cvdkjqmbicg3i8/contents/lib/python3.11/urllib/request.py", line 643, in http_error_default
    raise HTTPError(req.full_url, code, msg, hdrs, fp)
urllib.error.HTTPError: HTTP Error 403: Forbidden

```

then the likely cause is missing `--no-component-view`  from the flags.

The component view functionality depends on the mapping of directories
in a Chromium checkout to components in the issue tracker, so
misbehaves when run in a Dawn checkout. Fixing this would add
complexity upstream for a view that doesn't provide any useful
information in the report, since it would just tell you all of the
code in the Dawn repo belongs to either the Dawn or Tint component.
