# Running Clang-Tidy

* Add `"checkout_clang_tidy": True` to `.gclient` file in the `custom_vars`
  (or use `standalone-maximal.gclient` which includes this).

  ```py
  "custom_vars": {
    "checkout_clang_tidy": True,
  }
  ```
* `gclient sync`
* Set up a build directory for the configuration you want to run. It doesn't
  need to be built yet. This can be a native build or a cross-compiled build.
* Run `tools/run-tricium-clang-tidy.py --help` for instructions. Running the
  tool with `--default` takes about 12 minutes on a 10-core M1 Pro laptop and
  under 2.5 minutes on a 64-core (128-thread) cloud VM.
* This will output a json file with the full findings, and a summary file with a
  nicely-formatted list of results.

Tips:

* It's recommended to [set up clangd](./development-tips.md) and open a file
  in your IDE to see the errors in context (and to get an autofix button).
  In VSCode you can `less` the summary file to your terminal and then
  cmd-click/ctrl-click the lines to take you to the exact source line.
* You can use `clang-tidy` to process individual source files **and their headers** like so:
  `third_party/llvm-build/Release+Asserts/bin/clang-tidy -p out/debug SOURCE_FILES...`

  These must be source files - it isn't capable of processing header files directly.
  * Fixes can be applied with `--fix`. To fix just specific checks, add
    `--checks=-*,CHECKS,TO,FIX`.
    This is much slower to run over many files, as it's not parallelized.
    *If any file fails to compile, it won't apply any fixes to any file.*

    TODO(crbug.com/501491694): Investigate applying replacements from the
    `*-findings.json` file, using `clang-apply-replacements`. This would be
    somewhat limited because `tricium_clang_tidy_script.py` drops replacements
    when they're in a different file from the warning. Alternatively, see if
    there's a way to automate it via `clangd`.

## Workflow: Fixing a Given Check with Autofix

After you run `tools/run-tricium-clang-tidy.py` and get a list of findings (`clang-tidy-*-findings.json`), you can automatically apply fixes for specific checks that support autofix (such as `bugprone-parent-virtual-call` or `modernize-use-override`, see the [list of checks](https://clang.llvm.org/extra/clang-tidy/checks/list.html)) across all affected files using the helper script:

```bash
# Apply fixes for a specific check.
# The script will parse the `clang-tidy-xxx-findings.json`
# file, extract all affected files, and run clang-tidy with the --fix flag on them.
tools/apply-clang-tidy-fixes.py out/debug -f clang-tidy-xxx-findings.json -c bugprone-parent-virtual-call
```

Arguments & Options:
* `outdir`: The build directory (e.g., `out/debug`) (Required).
* `-c`, `--check`: The specific clang-tidy check name (Required).
* `-f`, `--findings`: Explicit path to a `findings.json` file (Defaults to the newest `clang-tidy-*-findings.json` in the current directory).

For additional info on running Clang-Tidy locally, see
[these instructions](https://chromium.googlesource.com/chromium/src/+/main/docs/clang_tidy.md).
