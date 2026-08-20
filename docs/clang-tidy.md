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
* This will output `out/*/clang-tidy-*-findings.json` with the full findings,
  and `out/*/clang-tidy-*-summary.txt` with a nicely-formatted list of results.
* Once you have findings, you can automatically apply fixes across any set of
  files for a specific check (if it
  [supports autofix](https://clang.llvm.org/extra/clang-tidy/checks/list.html)):
  Run `tools/apply-clang-tidy-fixes.py --help` for instructions.

Tips:

* It's recommended to [set up clangd](./development-tips.md) and open a file
  in your IDE to see the errors in context (and to get an autofix button).
  In VSCode you can `less` the summary file to your terminal and then
  cmd-click/ctrl-click the lines to take you to the exact source line.

For additional info on running Clang-Tidy locally, see
[these instructions](https://chromium.googlesource.com/chromium/src/+/main/docs/clang_tidy.md).
