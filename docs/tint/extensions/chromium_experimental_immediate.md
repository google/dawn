# Chromium Experimental immediate

The `chromium_experimental_immediate` extension adds support for immediate global variables to WGSL.
immediate data are small amounts of data that are passed to the shader and are expected to be more lightweight to set / modify than uniform buffer bindings.
The concept of immediate comes from Vulkan but D3D12 has similar "root constants".
Metal doesn't have the same concept but immediate data can be efficiently implemented with the `setBytes` family of command encoder methods.

## Status

immediate data support in Tint is highly experimental and only meant to be used in internal transforms at this stage.
Specification work in the WebGPU group hasn't started.

## Pseudo-specification

This extension adds a new `immediate` address space that's only allowed on global variable declarations.
immediate variables must only contain 32bit data types (or aggregates of such types).
immediate variable declarations must not have an initializer.
It is an error for a entry point to statically use more than one `immediate` variable.

## Example usage

```
var<immediate> draw_id : u32;

@fragment fn main() -> @location(0) u32 {
    return draw_id;
}
```
