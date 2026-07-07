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
  tool with `--default` takes about 12 minutes on a 10-core M1 Pro laptop.
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

For additional info on running Clang-Tidy locally, see
[these instructions](https://chromium.googlesource.com/chromium/src/+/main/docs/clang_tidy.md).
