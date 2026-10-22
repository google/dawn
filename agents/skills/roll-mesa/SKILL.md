---
name: roll-mesa
description: >-
  Roll the third_party/mesa/src dependency in Dawn and update the custom GN and
  CMake Mesa Lavapipe build rules in third_party/mesa/. Use this skill when
  asked to roll, update, or bump the Mesa dependency or fix Mesa build issues
  after a roll.
---

# Roll Mesa Dependency

Use this skill to roll `third_party/mesa/src` in Dawn and update Dawn's custom
GN and CMake build rules in `third_party/mesa/`.

## Workflow

### 1. Verify `.gclient` Configuration

1. Read `.gclient` in the Dawn workspace root.
2. Verify that `"checkout_mesa": True` is set inside `custom_vars`.
3. If `checkout_mesa` is missing or not set to `True`, **stop and prompt the
   user** to set `"checkout_mesa": True` in `.gclient` (or ask if they would
   like you to update `.gclient` for them) so that `gclient sync` pulls
   `third_party/mesa/src`, `third_party/llvm-dev`, and `third_party/zstd`.

### 2. Verify `third_party/mesa/src` is Present

1. Check whether `third_party/mesa/src` exists and is populated (e.g., check for
   `third_party/mesa/src/meson.build`).
2. If `third_party/mesa/src` is missing or empty, **prompt the user** to run
   `gclient sync` (or ask permission to run `gclient sync` now) before
   proceeding.

### 3. Roll `third_party/mesa/src` in `DEPS`

1. Record the current `third_party/mesa/src` commit hash from `DEPS` so you can
   diff upstream Mesa changes later (`<old_hash>`).
2. **Ask the user** if they have already performed the roll / updated the
   `third_party/mesa/src` hash in `DEPS`.
3. If they have **not** updated it yet:
   - Run `roll-dep` (from `depot_tools`) to update the hash in `DEPS`:
     ```bash
     roll-dep third_party/mesa/src
     ```
     *(If the working tree has local changes or the user wants a specific
     commit, pass `--ignore-dirty-tree` and/or `--roll-to <commit>` as
     appropriate, or update the hash in `DEPS` directly if requested.)*
4. Record the new `third_party/mesa/src` commit hash from `DEPS` (`<new_hash>`).

### 4. Sync the Updated Mesa Checkout

Run `gclient sync` to check out the updated `third_party/mesa/src` revision:

```bash
gclient sync
```

### 5. Build and Fix the GN Mesa Build

1. **Check the GN build directory (`out/active` by default)**:
   - Read `out/active/args.gn` and check that both of the following args are
     enabled:
     ```gn
     tint_build_mesa = true
     tint_build_fuzzer_vulkan_support = true
     ```
   - If `out/active/args.gn` does not exist or does not have both args set to
     `true`, **ask the user** whether they want to update `out/active/args.gn`
     or use a different GN build directory.
2. **Run the GN build**:
   ```bash
   autoninja -C <gn_build_dir> third_party/mesa:mesa tint_wgsl_mesa_fuzzer tint_ir_mesa_fuzzer
   ```
3. **Analyze GN build results**:
   - If the build fails, inspect the errors and compare upstream Mesa build
     changes between the old and new commits:
     ```bash
     git -C third_party/mesa/src diff <old_hash>..<new_hash> -- "**/meson.build" "meson_options.txt"
     ```
   - **Check for external dependency requirements**: Determine whether the
     failure requires updating another external dependency, such as
     `third_party/llvm-dev` (e.g., Mesa requires a newer LLVM version or newly
     added LLVM C++/C APIs not present in `third_party/llvm-dev`),
     `third_party/zstd`, or Python packages in `vpython.toml`.
     - **STOP CONDITION**: If another dependency (`llvm-dev`, `zstd`, etc.) must
       be updated to roll Mesa, **advise the user of the exact details and do
       not proceed**, as updating those prebuilt packages requires manual steps
       outside the scope of this skill.
   - **Mitigate within `third_party/mesa/`**: If the failure is caused by
     upstream Mesa source/codegen/flag changes that can be resolved in Dawn's
     custom GN rules, update:
     - `third_party/mesa/mesa_sources.gni`: Add, remove, or rename source files
       in the source lists, or update `mesa_common_defines` /
       `mesa_common_cflags*`.
     - `third_party/mesa/BUILD.gn`: Update `_mesa_action` / shader codegen
       targets, arguments, inputs, outputs, include directories, or
       `_mesa_source_set` dependencies, then run
       `gn format third_party/mesa/BUILD.gn third_party/mesa/mesa_sources.gni`.
     - `third_party/mesa/mesa_llvm_sysroot_compat.cc` or
       `third_party/mesa/mesa_action.py` if needed.
   - Re-run `autoninja` until `third_party/mesa:mesa`, `tint_wgsl_mesa_fuzzer`,
     and `tint_ir_mesa_fuzzer` compile and link cleanly.
4. **Test the GN build**: Run a smoke test with `tint_wgsl_mesa_fuzzer` to
   confirm `libvulkan_lvp.so` loads via `lvp_icd.json` and validates a WGSL
   shader cleanly:
   ```bash
   ./<gn_build_dir>/tint_wgsl_mesa_fuzzer --vk-icd=./<gn_build_dir>/lvp_icd.json --verbose test/tint/out_of_order_decls/alias/struct.wgsl
   ```

### 6. Build and Fix the CMake Mesa Build

1. **Sync any GN rule changes to CMake**: If you modified
   `third_party/mesa/mesa_sources.gni` or `third_party/mesa/BUILD.gn` in Step 5,
   apply the corresponding updates to `third_party/mesa/mesa_sources.cmake` and
   `third_party/mesa/CMakeLists.txt`.
2. **Check the CMake build directory (`out/cmake` by default)**:
   - Check `out/cmake/CMakeCache.txt` to verify that `TINT_BUILD_MESA:BOOL=ON`
     and `TINT_BUILD_FUZZER_VULKAN_SUPPORT:BOOL=ON` (along with
     `TINT_BUILD_FUZZERS:BOOL=ON`) are enabled.
   - If `out/cmake` does not exist or those options are not enabled, **prompt
     the user** to ask whether to enable them in `out/cmake` (via
     `cmake -B out/cmake -DTINT_BUILD_MESA=ON -DTINT_BUILD_FUZZER_VULKAN_SUPPORT=ON -DTINT_BUILD_FUZZERS=ON`)
     or use a different CMake build directory.
3. **Run the CMake build**:
   ```bash
   cmake --build <cmake_build_dir> --target mesa tint_wgsl_mesa_fuzzer tint_ir_mesa_fuzzer
   ```
4. **Analyze CMake build results**:
   - If the CMake build fails, determine whether the failure can be mitigated
     with changes to `third_party/mesa/mesa_sources.cmake` or
     `third_party/mesa/CMakeLists.txt`.
   - **STOP CONDITION**: Just as in the GN step, if the CMake build failure
     indicates that another dependency (`llvm-dev`, `zstd`, etc.) requires an
     update, **inform the user with the details and do not proceed**.
   - Re-run `cmake --build` until `mesa`, `tint_wgsl_mesa_fuzzer`, and
     `tint_ir_mesa_fuzzer` build cleanly with zero undefined symbols.
5. **Test the CMake build**: Run a smoke test with the CMake
   `tint_wgsl_mesa_fuzzer` binary:
   ```bash
   ./<cmake_build_dir>/tint_wgsl_mesa_fuzzer --vk-icd=./<cmake_build_dir>/lvp_icd.json --verbose test/tint/out_of_order_decls/alias/struct.wgsl
   ```
