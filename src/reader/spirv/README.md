# SPIR-V Reader

This component translates SPIR-V written for Vulkan into the Tint AST.

The SPIR-V reader entry point is `tint::reader::spirv::Parser`, which
implements the Reader interface in `tint::reader::Reader`.

It's usable from the Tint command line:

    # Dump the Tint AST after reading SPIR-V.
    tint --dump-ast --parse-only a.spv

    # Translate SPIR-V into WGSL.
    tint --format wgsl a.spv

## Validation

The SPIR-V module must pass validation for the `SPV_ENV_WEBGPU_0` target
environment in SPIRV-Tools.

That set of rules is _experimental_ and was originally intended
to constraint SPIR-V modules being ingested directly by the WebGPU API.
Those rules are now too restrictive, because some amount of sanitization
and normalization occurs during translation from SPIR-V to WGSL.
The validation rules will be relaxed at some point TBD.

Generally, validation of _functionality_ used will remain, e.g. WebGPU currently
does not support `CullDistance` or subgroup operations.

However, detailed rules about the form of SPIR-V can be relaxed, e.g. the
requirement to use SPIR-V 1.3, and restrictive rules about statically unreachable
code.

## Feedback

Please file issues at https://crbug.com/tint, and put `spirv-reader` in the issue title.

Outstanding issues can be found by using the `spirv-reader` label in the Chromium project's
bug tracker: https://bugs.chromium.org/p/tint/issues/list?q=label:spirv-reader
