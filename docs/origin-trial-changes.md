# Tint changes during Origin Trial

## Changes for M97

### Breaking Changes

* Deprecated `modf()` and `frexp()` builtin overloads that take a pointer second parameter have been removed.
* Deprecated texture builtin functions that accepted a `read` access controlled storage texture have been removed.
* Storage textures must now only use the `write` access control.

### Deprecated Features

* The `ignore()` builtin has been replaced with phony-assignment. [gpuweb#2127](https://github.com/gpuweb/gpuweb/pull/2127)

### New Features

* `any()` and `all()` now support a `bool` parameter. These simply return the passed argument. [tint:1253](https://crbug.com/tint/1253)
* Call statements may now include functions that return a value (`ignore()` is no longer needed).
* The `interpolate(flat)` attribute can now be specified on integral user-defined IO. It will eventually become an error to define integral user-defined IO without this attribute.
* Matrix construction from scalar element values is now supported.

### Fixes

* Swizzling of `vec3` types in `storage` and `uniform` buffers has been fixed for Metal 1.x. [tint:1249](https://crbug.com/tint/1249)
* Calling a function that returns an unused value no longer produces an FXC compilation error. [tint:1259](https://crbug.com/tint/1259)

## Changes for M95

### New Features

* The size of an array can now be defined using a non-overridable module-scope constant
* The `num_workgroups` builtin is now supported.

### Fixes

* Hex floats: now correctly errors when the magnitude is non-zero, and the exponent would cause overflow. [tint:1150](https://crbug.com/tint/1150), [tint:1166](https://crbug.com/tint/1166)
* Identifiers beginning with an underscore are now correctly rejected.  [tint:1179](https://crbug.com/tint/1179)
* `abs()` fixed for unsigned integers on SPIR-V backend   [tint:1179](https://crbug.com/tint/1194)
