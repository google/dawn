# Tint: Integer Range Analysis

This document provides a high-level overview of Integer Range Analysis in Tint. It is intended for
developers who want to know whether a shader can benefit from the analysis, or who want to
modify the Integer Range Analysis pass at
[integer_range_analysis.cc](../../src/tint/lang/core/ir/analysis/integer_range_analysis.cc).

## Overview

Integer Range Analysis computes a conservative, inclusive range `[min_bound, max_bound]` for scalar
`i32` and `u32` values in the Tint IR. Each range is stored in an `IntegerRangeInfo`, which holds a
`SignedIntegerRange` (for `i32`) or an `UnsignedIntegerRange` (for `u32`). If the analysis can't
compute the range of a value, it returns an `IntegerRangeInfo` containing `std::monostate`, and
`IntegerRangeInfo::IsValid()` returns `false`.

The development of Integer Range Analysis can be tracked at https://crbug.com/348701956.

## Where the analysis is used

Currently, Integer Range Analysis is only used in the `Robustness` transform
([robustness.cc](../../src/tint/lang/core/ir/transform/robustness.cc)), which uses the analysis
when `RobustnessConfig::use_integer_range_analysis` is set to `true`. In Tint, the analysis is
enabled by default in the SPIR-V, HLSL, MSL and GLSL writers, and can be disabled with the
`disable_integer_range_analysis` writer option.

In Dawn, every backend sets `disable_integer_range_analysis` from the device toggle
`enable_integer_range_analysis_in_robustness`. The default value of this toggle comes from the
platform feature `kWebGPUEnableRangeAnalysisForRobustness`. Dawn's default platform implementation
enables this feature, so the toggle is enabled by default. Platform implementations (such as in
Chromium) and explicit toggle settings can override this default.

Robustness uses the analysis in these places:

| Operation | Proof to skip robustness transform |
| --- | --- |
| Indexing a fixed-size array, matrix column, or vector element | The index range is non-negative and its maximum is at most `element_count - 1`. |
| Loading or storing a vector element through a pointer | The index range is non-negative and its maximum is at most `vector_width - 1`. |
| Subgroup matrix load/store stride | The stride range is non-negative and its minimum is at least the required stride. |
| Subgroup matrix load/store into a fixed-size array | Both `offset` and `stride` have non-negative ranges that prove the entire matrix access fits in the array. |

For array, matrix column or vector element indexing, the maximum allowed index must be an IR
constant. A runtime-sized `array<T>` has no constant length, so the analysis never removes its
index clamp.

For subgroup matrices, `offset` and `stride` are counted in elements of the array being accessed.
The minimum-stride clamp (`stride = max(stride, stride_min)`) is checked independently of the array
bounds check, so it can be skipped even on runtime-sized arrays.
For fixed-size arrays, skipping the bounds check requires both `offset` and `stride` to have known
non-negative ranges. Let
- `offset_max` be the maximum value of `offset`.
- `stride_max` be the maximum value of `stride`.
- `stride_min` be the minimum valid stride in array elements:
  `max((minor_dim * matrix_element_size) / array_stride, 1)`, where `minor_dim` is the number of
  rows when the builtin's majorness argument is `col_major`, or the number of columns when it is
  `row_major`.
- `major_dim` be the number of major slices (columns when the majorness is `col_major`, rows when
  it is `row_major`).

The majorness is specified by the shader author in the `subgroupMatrixLoad`/`subgroupMatrixStore`
call, and describes how the matrix is laid out in the array. It is not a backend layout detail.

The fixed-array proof is:

```text
offset_max + max(stride_max, stride_min) * (major_dim - 1) + stride_min <= array_length
```

Texture builtins (`textureLoad`, `textureDimensions`) and `bufferView`/`bufferArrayView` calls
don't use the analysis, so their arguments are always clamped when robustness is required. For
texture builtins and for buffer views on runtime-sized buffers, the limits (texture dimensions,
mip level count, array layer count, sample count, and buffer length) are only known at runtime, so
the analysis can't prove an access in bounds. For buffer views on fixed-size `buffer<N>`, the
length is a constant, so the analysis could be used, but this isn't implemented yet.

Some platforms skip Tint's software robustness checks for selected bindings because the platform
already handles out-of-bounds accesses. No clamp is emitted for these bindings in the first place,
so the analysis has nothing to remove:

- D3D12 marks both read-only and read-write storage-buffer bindings **without dynamic offsets** as
  ignored by the Robustness transform (for subgroup matrix calls on these bindings, array bounds
  clamping is skipped, though the minimum-stride clamp still runs unless proven safe by the
  analysis).
- Vulkan skips index clamping on runtime-sized arrays in storage buffers when the toggle
`VulkanUseBufferRobustAccess2` is enabled. For runtime-sized arrays in uniform buffers, skipping
index clamping requires both this toggle and `robustUniformBufferAccessSizeAlignment <= 16`.
- Vulkan skips subgroup matrix array bounds clamping on storage buffers when the toggle
`VulkanUseCooperativeMatrixRobustBufferAccess` is enabled (the minimum-stride clamp still runs
unless proven safe by the analysis).

## Sources of known ranges

These are the values with ranges that can be computed directly. We can use these ranges to compute
the ranges of the expressions that contain these values.

### Integer constants
A scalar `i32` or `u32` constant value `c` has the range `[c, c]`.

### Built-in input values
| Built-in | Range | Requirement |
|---|---|---|
| `local_invocation_index` | `[0, x * y * z - 1]` | The workgroup size `(x, y, z)` is constant. |
| `local_invocation_id` | component `i`: `[0, workgroup_size[i] - 1]` | The workgroup size is constant. |
| `subgroup_invocation_id` | `[0, S - 1]` | The function has a constant `@subgroup_size(S)` with `S > 0`. |
| `subgroup_size` | `[S, S]` | The function has a constant `@subgroup_size(S)` with `S > 0`. |

All overrides should already have been replaced by constants before the analysis runs, so the
analysis asserts that the workgroup size is constant. The built-in must be a direct function
parameter. Other built-ins and user-defined parameters have no range.

A `vec3<u32>` parameter such as `local_invocation_id` only has a range through an `access` with a
single constant index, for example `lid.x` or `lid[1]`.

```wgsl
@compute @workgroup_size(16, 8, 1)
fn main(@builtin(local_invocation_id) lid : vec3u,
        @builtin(local_invocation_index) idx : u32) {
  let a = lid.x;  // [0, 15]
  let b = lid.y;  // [0, 7]
  let c = idx;    // [0, 127]
}
```

### Loop control variables of simple for-loops
The analysis can compute the range of the loop control variable `i` of a for-loop like this:

```wgsl
for (var i = <init>; <i compared against a constant>; <i increased or decreased by a constant>) {
  // Loop body, where i cannot be updated or re-assigned to another value.
  ...
}
```

The range applies to the `var`, so every `load` of `i` in the loop body or in the continuing
block gets the range. Below requirements are all defined with Tint IR instead of WGSL as
the analysis is applied on Tint IR.

#### Requirements

Below are the requirements for the supported for-loop in Integer Range Analysis. All of the
following must hold. The requirements are applied on the Tint IR that Tint produces for a WGSL
`for` statement, and the analysis matches the shape of that IR exactly.

**Initializer**
- The control variable must be a scalar `i32` or `u32` `var` with an initializer.
- The initializer must have a known scalar integer range.
- The `var` must immediately precede the terminating `next_iteration` in the loop initializer
block.

**Continuing Clause**
- The continuing block must contain exactly 4 instructions: `load i`, an `add` or `sub`, `store i`
and `next_iteration`.
- The update must add a positive integer constant to the variable (`i + step` or `step + i`), or
subtract a positive integer constant from it (`i - step`), where `step` is an IR constant (a
literal or `const`; an `override` also qualifies because it has been replaced by a constant before
the analysis runs, but a `let` does not). Adding or subtracting a negative constant (such as
`i += -1`) is not supported.

**Comparison and direction**
- The loop body must start with `load i`, immediately followed by the comparison, immediately
followed by an `if` on the comparison result. Its true branch must terminate with `exit_if`, and
its false branch must start with `exit_loop`. So the condition must be a single comparison:
`for (...; i < 10 && ok; ...)` is not supported.
- The condition must compare the variable with an IR integer constant (a literal, `const`, or
`override` replaced by a constant, but not a `let`) using `<`, `<=`, `>`, or `>=`.
- The loop is rejected when the comparison is always false (for example `i < 0u` or
`i > 2147483647`) or always true (for example `i >= 0u` or `i <= 4294967295u`). An always-true
comparison means the loop can't exit through it, so `i` can wrap and take any value. Only
comparisons against the minimum or maximum value of the type are detected this way.

Once the comparison is accepted, the analysis normalizes it to `i <= C'` or `i >= C'`:
| Original | Rewritten |
|---|---|
| `i < C` or `C > i` | `i <= C - 1` |
| `i <= C` or `C >= i` | `i <= C` |
| `i > C` or `C < i` | `i >= C + 1` |
| `i >= C` or `C <= i` | `i >= C` |

- `i <= C` is supported only when `i` is increasing (`add`).
- `i >= C` is supported only when `i` is decreasing (`sub`).
- If the loop body can never run (`init_min > C` for increasing loops, `init_max < C`
for decreasing loops), no range is computed.
- The last value that passes the check plus (or minus) `step` must still fit in the type of `i`.
Otherwise `i` could overflow and still pass the check, so no range is computed.

**Loop Body**
- The control variable can only be used through `load` instruction, and only one `store` in the
continuing block. Any other use makes the loop unsupported, for example a store to `i` in the body
or passing `&i` to a function.

#### Resulting range

Let `[init_min, init_max]` be the range of the initializer.

- **Increasing** (`i <= C'`, `i += step`): the range is `[init_min, index_max]`. `index_max` is the
  largest value `init + (k * step) <= C'` over all `init` in `[init_min, min(init_max, C')]`. With
  `step == 1`, or when some initial value can land exactly on `C'`, `index_max == C'`.
- **Decreasing** (`i >= C'`, `i -= step`): the range is `[index_min, init_max]`. `index_min` is the
  smallest value `init - (k * step) >= C'` over all `init` in `[max(init_min, C'), init_max]`.

#### Examples

```wgsl
@compute @workgroup_size(64)
fn main(@builtin(local_invocation_id) lid : vec3u) {
  for (var i = 0; i < 10; i++) {}                // i: [0, 9]
  for (var i = 0u; i <= 16u; i += 6u) {}         // i: [0, 12]
  for (var i = 10; i > 0; i--) {}                // i: [1, 10]
  for (var i = 10u; i >= 1u; i -= 2u) {}         // i: [2, 10]
  for (var i = lid.x; i < 256u; i += 64u) {}     // i: [0, 255]
  for (var i = 0u; i < 4u; i++) {
    for (var j = i * 2u; j < 8u; j++) {}         // i: [0, 3], j: [0, 7]
  }
}
```

Unsupported loops:

```wgsl
for (var i = 10u; i >= 0u; i--) {}        // Comparison is always true; i wraps around.
for (var i = 0; i > -10; i++) {}          // Direction doesn't match the comparison.
for (var i = 10; i > 0; i += -1) {}       // Step is negative; use `i -= 1` or `i--` instead.
for (var i = 0u; i < n; i++) {}           // Bound is not an IR constant (even if `n` is a `let`).
for (var i = 0; i < 10; i += 2) { i++; }  // i is modified in the loop body.
for (var i = 0; i < 10; i *= 2) {}        // Update is not add/sub of a constant.
var i = 0;
loop {                                    // Not the for-loop pattern above.
  if (i >= 10) { break; }
  continuing { i++; }
}
```

## Derived ranges

Integer Range Analysis computes the range of a value from its operands as follows.

### `let`

`let x = e;` has the same range as `e`.

### `load`

A `load` has a range only if it loads from a `var` that has one. Right now that means only loop
control variables. Other function-scope or module-scope variables have no range.

### `access`

An `access` has a range only when:

- The object is a function parameter that has a range, which is one of the built-ins listed above.
- It has exactly one index, and that index is a constant.

### `convert`

Only scalar conversions between `i32` and `u32` are supported:

- `u32(x)` with `x: i32`: requires `min_bound >= 0`. The range is unchanged. Otherwise the result
  has no range.
- `i32(x)` with `x: u32`: requires `max_bound <= 2147483647`. The range is unchanged. Otherwise the
  result has no range.

### Binary operators

For every operator below, both operands need a valid range, except `%` (see below). Any bound that
would overflow or underflow the result type makes the result range-less.

| Operator | Result | Extra requirements |
|---|---|---|
| `a + b` | `[a.min + b.min, a.max + b.max]` | |
| `a - b` | `[a.min - b.max, a.max - b.min]` | For `u32`, `a.min >= b.max`. |
| `a * b` | `[a.min * b.min, a.max * b.max]` | For `i32`, both operands are non-negative. |
| `a / b` | `[a.min / b.max, a.max / b.min]` | `b.min > 0`. For `i32`, `a.min >= 0`. |
| `a << b` | `[a.min << b.min, a.max << b.max]` | `b.max < 32`. For `i32`, `a.min >= 0`. |
| `a >> b` | `[a.min >> b.max, a.max >> b.min]` | `b.max < 32`. For `i32`, `a.min >= 0`. |
| `a % b` | See below. | `b` has a single-value range `[c, c]` with `c > 0`. |

**Modulo.** `a % b` needs `b` to have a single-value range `[c, c]` with `c > 0`. If `a` has no
range, the analysis uses the full range of its type. For `i32`, `a` must be non-negative, so an
`i32` value without a range is not supported. Then:

- If `a.min / c == a.max / c`, the result is `[a.min % c, a.max % c]`.
- Otherwise, the result is `[0, c - 1]`.

```wgsl
// `buf` is a storage buffer, so loads from it have no range.
// With @workgroup_size(64), lid.x is [0, 63].
let x : u32 = buf[0];        // No range.
let r = x % 16u;             // [0, 15]
let s = (lid.x + 3u) % 4u;   // [3, 66] % 4 => [0, 3]
let t = lid.x % 128u;        // [0, 63]
```

Other binary operators (`&`, `|`, `^`, comparisons, and so on) are not supported.

### Built-in function calls

Only `min` and `max` with scalar integer results are supported. If one argument has no range, the
analysis uses the full range of its type for that argument. At least one argument must have a
range.

| Built-in | Result |
|---|---|
| `min(a, b)` | `[min(a.min, b.min), min(a.max, b.max)]` |
| `max(a, b)` | `[max(a.min, b.min), max(a.max, b.max)]` |

For `min` and `max`, if the result covers the whole domain of the type, it is treated as having no
range.

```wgsl
// `buf` is a storage buffer, so loads from it have no range.
let x : i32 = buf[0];          // No range.
let a = min(x, 15);            // [-2147483648, 15]
let b = max(x, 0);             // [0, 2147483647]
let c = min(max(x, 0), 15);    // [0, 15]
let y : u32 = buf[1];          // No range.
let d = min(y, 63u);           // [0, 63]
```

## Unsupported patterns

These patterns are not supported yet. They are candidates for future improvements to the analysis.
Recognition happens on the IR that reaches the Robustness transform, so the IR is what decides
whether a pattern is recognized (see [Checking a shader](#checking-a-shader)). A workaround is
given where one exists:

- **More built-in functions.** Only `min` and `max` are supported. `clamp`, `abs`,
  `countOneBits`, `firstLeadingBit` and other integer built-ins produce no range. Use
  `min(max(x, lo), hi)` instead of `clamp(x, lo, hi)`.
- **Bitwise operators.** `&`, `|`, `^` and `~` produce no range, even for common masks like
  `x & 15u`. Use `x % 16u` instead, which is recognized even when `x: u32` has no range.
- **Other operators and instructions.** Unary minus, `bitcast`, `select` and comparisons produce
  no range.
- **Refinement from conditions.** A condition such as `if (x < 16u)` does not refine `x`'s range
  inside the branch. The supported loop pattern is handled separately.
- **Relationships between values.** Operand ranges are combined independently. For example, if
  `x` has range `[0, 15]`, `x - x` is not inferred to be `[0, 0]`; for `u32`, the subtraction
  rule rejects it because the independently combined bounds could underflow.
- **Signed operands that may be negative.** `*`, `/`, `<<`, `>>` and `%` on `i32` require
  non-negative operands.
- **Non-constant modulo divisors.** `a % b` requires `b` to have a single-value range.
- **Vector values.** Only scalar `i32` and `u32` values have ranges. Vector arithmetic,
  and components of vectors other than built-in parameters (for example `let v = lid; v.x`),
  produce no range. Access built-in parameters directly (`lid.x`) instead.
- **Variables other than loop control variables.** Loads from other function-scope or
  module-scope variables produce no range.
- **Other built-in values.** For example `global_invocation_id`, `workgroup_id` and
  `num_workgroups` have no range.
- **More general loops.** `loop` and `while` statements, compound loop conditions, non-constant
  loop bounds (including `let` bindings), negative step constants (`i += -1`), and continuing
  blocks that do anything besides a single `add` or `sub` are not analyzed.
- **Range-based checks in robustness.** Texture builtins, `bufferView`/`bufferArrayView` calls and
  runtime-sized arrays don't use the analysis.

## Checking a shader

The rules in this document are applied to the Tint IR that reaches the Robustness transform, not to
the WGSL source. To check whether a shader matches them, check that IR. Dump it with the `tint`
command-line tool:

```sh
tint --format hlsl --dump-ir shader.wgsl
```

`--dump-ir` prints the IR before every transform. The section titled
`IR dump before core.Robustness:` is exactly the input to Integer Range Analysis. Use `--format`
to select the backend (`spirv`, `hlsl`, `hlsl-fxc`, `msl` or `glsl`), because each backend runs a
different set of transforms before Robustness. `--dump-ir` requires a build with IR dumping
enabled (`TINT_ENABLE_IR_DUMPING` in CMake, on by default; `tint_enable_ir_dumping` in GN, on by
default in standalone Dawn builds).

Pass the same configuration that the shader is compiled with:

- `--entry-point <name>` to keep only the entry point being compiled.
- `--overrides NAME=VALUE,...` to give values to `override` declarations. Overrides are replaced
  by constants before Robustness, so their values can change the computed ranges (for example
  `@workgroup_size` or a loop bound).

When the IR is the input, an AI tool can check the shader against these rules. Give it this
document and the `IR dump before core.Robustness:` section, and ask it to check each index operand
of an `access` or a `load_vector_element`/`store_vector_element` that targets a fixed-size array,
matrix or vector.
