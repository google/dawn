# Tips for developing Dawn code

## Build/run as much stuff as possible locally, to find bugs before they hit CQ

- Use `scripts/standalone-maximal.gclient` instead of `standalone.gclient`.
- Set the `dawn_maximal = true` GN arg to default to building as many targets
  as possible given your host OS.
- Build all targets (target `all`, which is the default if none is specified).
- Develop on Linux or Mac, and set up build directories that cross-compile for
  other OSes (see [building.md](building.md)).
  - `dawn_maximal = true` is optional but still useful for cross-compile builds.
- Also note you can run `gn gen --check` to locally run the presubmit check
  that C++ includes match the build files.

## Set up your IDE to use clangd

- Do the steps above, so that clangd will know how to build most files.
- Do a full build in each build directory (to create generated files, etc.).
- Set `"custom_vars": { "checkout_clangd": True }` in `.gclient`
  (or just use `standalone-maximal.gclient`).
- Point your IDE to the binary at
  `third_party/llvm-build/Release+Asserts/bin/clangd`.
  - In VSCode: `clangd.path`
- Configure clangd: Paste the relevant text into `.clangd` and then update the
  `out/` paths in that file according to your needs.
  - Note the `PathMatch` paths aren't intended to be perfect, but please update
    this doc if you find platform-specific files that they miss.
  - If you have just one build directory, you can alternatively pass this as
    a clangd argument: `--compile-commands-dir=out/Debug`
    - In VSCode: `clangd.arguments`
  - For Chromium builds, take a look at
    [`chrome-remote-index`](https://github.com/clangd/chrome-remote-index/blob/main/docs/index.md).

For Mac/Linux hosts:

```yml
# This first section applies by default. Additional "fragments" below *also*
# apply, and may overwrite configs from less-specific fragments.
# Docs: https://clangd.llvm.org/config
CompileFlags:
  CompilationDatabase: out/Debug
  Add:
    - "-ferror-limit=0"
    - "-fsafe-buffer-usage-suggestions"
    # Additional personal configs can go here OR in your user-wide
    # `clangd/config.yml` (https://clangd.llvm.org/config#files). Suggestions:
    # - `--header-insertion=never` if you find yourself accidentally adding
    #   random headers when autocomplete goes sideways
---
If:
  PathMatch:
    - "src/dawn/native/d3d.*"
    - "src/.*_[Ww]in.*"
    - "src/.*_[Dd]3[Dd].*"
CompileFlags:
  CompilationDatabase: out/Debug-win
```

For Linux hosts, also add this:

```yml
---
If:
  PathMatch:
    - "src/dawn/native/metal/.*"
    - "src/.*_[Mm]ac.*"
    - "src/.*_[Aa]pple.*"
    - "src/.*_[Mm]etal.*"
CompileFlags:
  CompilationDatabase: out/Debug-mac
```
