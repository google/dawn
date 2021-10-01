# Tint changes during Origin Trial

## Changes for M95

### New Features

* The size of an array can now be defined using a non-overridable module-scope constant
* The `num_workgroups` builtin is now supported.

### Fixes

* Hex floats: now correctly errors when the magnitude is non-zero, and the exponent would cause overflow. [tint:1150](https://crbug.com/tint/1150), [tint:1166](https://crbug.com/tint/1166)
* Identifers beginning with an underscore are now correctly rejected.  [tint:1179](https://crbug.com/tint/1179)
* `abs()` fixed for unsigned integers on SPIR-V backend   [tint:1179](https://crbug.com/tint/1194)
