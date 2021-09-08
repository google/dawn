# Tint changes during Origin Trial

## Changes for M95

### New Features
* The size of an array can now be defined using a non-overridable module-scope constant

### Fixes
* Hex floats: issue an error when the magnitude is non-zero, and the exponent would cause
    overflow. https://crbug.com/tint/1150 https://crbug.com/tint/1166

