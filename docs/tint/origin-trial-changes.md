# Tint changes during Origin Trial

## Changes for M103

### New features

* Produce warnings for when calling barriers, textureSample, and derivative
builtins in non-uniform control flow [tint:880](crbug.com/tint/880)

## Changes for M102

### New Features

* Parentheses are no longer required around expressions for if and switch statements [tint:1424](crbug.com/tint/1424)
* Compound assignment statements are now supported. [tint:1325](https://crbug.com/tint/1325)
* Postfix increment and decrement statements are now supported. [tint:1488](crbug.com/tint/1488)
* The colon in case statements is now optional. [tint:1485](crbug.com/tint/1485)

### Breaking changes

* Struct members are now separated by commas. [tint:1475](crbug.com/tint/1475)
* The `@block` attribute has been removed. [tint:1324](crbug.com/tint/1324)
* The `@stride` attribute has been removed. [tint:1381](crbug.com/tint/1381)
* Attributes using `[[attribute]]` syntax are no longer supported. [tint:1382](crbug.com/tint/1382)
* The `elseif` keyword is no longer supported. [tint:1289](crbug.com/tint/1289)

### Deprecated Features

* The `smoothStep()` builtin has been renamed to `smoothstep()`. [tint:1483](crbug.com/tint/1483)

## Changes for M101

### New Features

* Tint now supports unicode identifiers. [tint:1437](crbug.com/tint/1437)

### Breaking changes

* The `isNan()`, `isInf()`, `isFinite()`, and `isNormal()` builtins have been removed. [tint:1312](https://crbug.com/tint/1312)

## Changes for M100

### Breaking changes

* The `@interpolate(flat)` attribute must now be specified on integral user-defined IO. [tint:1224](crbug.com/tint/1224)
* The `ignore()` intrinsic has been removed. Use phoney-assignment instead: `ignore(expr);` -> `_ = expr;`.
* `break` statements in `continuing` blocks are now correctly validated.

### New Features

* Module-scope declarations can now be declared in any order. [tint:1266](crbug.com/tint/1266)
* The `override` keyword and `@id()` attribute for pipeline-overridable constants are now supported, replacing the `@override` attribute. [tint:1403](crbug.com/tint/1403)

## Changes for M99

### Breaking changes

Obviously infinite loops (no condition, no break) are now a validation error.

### Deprecated Features

The following features have been deprecated and will be removed in M102:

* The `[[block]]` attribute has been deprecated. [tint:1324](https://crbug.com/tint/1324)
* Attributes now use the `@decoration` syntax instead of the `[[decoration]]` syntax. [tint:1382](https://crbug.com/tint/1382)
* `elseif` has been replaced with `else if`. [tint:1289](https://crbug.com/tint/1289)
* The `[[stride]]` attribute has been deprecated. [tint:1381](https://crbug.com/tint/1381)

### New Features

* Vector and matrix element type can now be inferred from constructor argument types. [tint:1334](https://crbug.com/tint/1334)
* Added builtins `degrees()` and `radians()` for converting between degrees and radians. [tint:1329](https://crbug.com/tint/1329)
* `let` arrays and matrices can now be dynamically indexed. [tint:1352](https://crbug.com/tint/1352)
* Storage and Uniform buffer types no longer have to be structures. [tint:1372](crbug.com/tint/1372)
* A struct declaration does not have to be followed by a semicolon. [tint:1380](crbug.com/tint/1380)

### Fixes

* Fixed an issue where for-loops that contain array or structure constructors in the loop initializer statements, condition expressions or continuing statements could fail to compile. [tint:1364](https://crbug.com/tint/1364)

## Changes for M98

### Breaking Changes

* Taking the address of a vector component is no longer allowed.
* Module-scope declarations can no longer alias a builtin name. [tint:1318](https://crbug.com/tint/1318)
* It is now an error to call a function either directly or transitively, from a loop continuing block, that uses `discard`. [tint:1302](https://crbug.com/tint/1302)

### Deprecated Features

* The `isNan()`, `isInf()`, `isFinite()` and `isNormal()` builtins has been deprecated and will be removed in M101. [tint:1312](https://crbug.com/tint/1312)

### New Features

* New texture gather builtins: `textureGather()` and `textureGatherCompare()`. [tint:1330](https://crbug.com/tint/1330)
* Shadowing is now fully supported. [tint:819](https://crbug.com/tint/819)
* The `dot()` builtin now supports integer vector types.
* Identifiers can now start with a single leading underscore.  [tint:1292](https://crbug.com/tint/1292)
* Control flow analysis has been improved, and functions no longer need to `return` if the statement is unreachable. [tint:1302](https://crbug.com/tint/1302)
* Unreachable statements now produce a warning instead of an error, to allow WGSL code to be updated to the new analysis behavior. These warnings may become errors in the future [gpuweb#2378](https://github.com/gpuweb/gpuweb/issues/2378)

### Fixes

* Fixed an issue where using a module-scoped `let` in a `workgroup_size` may result in a compilation error. [tint:1320](https://crbug.com/tint/1320)

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
* `abs()` fixed for unsigned integers on SPIR-V backend

## Changes for M95

### New Features

* The size of an array can now be defined using a non-overridable module-scope constant
* The `num_workgroups` builtin is now supported.

### Fixes

* Hex floats: now correctly errors when the magnitude is non-zero, and the exponent would cause overflow. [tint:1150](https://crbug.com/tint/1150), [tint:1166](https://crbug.com/tint/1166)
* Identifiers beginning with an underscore are now correctly rejected.  [tint:1179](https://crbug.com/tint/1179)
