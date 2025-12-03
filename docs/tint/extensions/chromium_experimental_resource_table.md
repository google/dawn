# Chromium Experimental Resource Table

The `chromium_experimental_resource_table` is an experimental extension that provides different
prototype options to support the WebGPU bindless prototypes.

# Status

This extension is experimental and the syntax is being discussed. No official WebGPU specification
has been written yet.

# Availability

| Platform | Notes |
| SPIR-V | Requires `SPV_EXT_descriptor_indexing` extension and `RuntimeDescriptorArray` capability |
| HLSL | Supported for DXC, not FXC |
| MSL | Supported (TODO determine if there are Metal version restrictions) |
| GLSL | Not supported |

Due to limited availability, this will need an `enable` statement to be used. For this experimental
extension it would be `enable chromium_experimental_resource_table`.

# Specification

## Resource table

The concept of a `Resource Table` is added to WGSL. This is not a type which can be written, or
an address space, but a data table available to use with the `getResource` and `hasResource`
methods. The table is made available by the system and is just accessible.

### `hasResource`
```
@must_use fn hasResource<T>(index: I) -> bool
```
* `I` is an `i32` or `u32`
* `T` is a format-less storage texture (e.g. `texture_storage_2d<f32>`), sampled texture, multisampled
  texture, or depth texture.

`hasResource` returns true if the item a `index` of the resource table is of type `T`.

### `getResource`
```
@must_use fn getResource<T>(index: I) -> T
```
* `I` is an `i32` or `u32`
* `T` is a format-less storage texture (e.g. `texture_storage_2d<f32>`), sampled texture, multisampled
  texture, or depth texture.

`getResource` returns the value in the resource table at `index` of type `T`.

If `index` is outside the bounds of the resource table then a default value of type `T` will be
returned. If the item at `index` is not of type `T` then a default value of type `T` is returned.
Essentially, a value is always returned, it may just be a synthensized default value.

# Example usage

## Resource Table
```
enable chromium_experimental_resource_table;

const kHouseTexture = 0u;

@fragment fn fs() {
    if (hasResource<texture_2d<f32>>(kHouseTexture)) {
        let tex = textureLoad(getResource<texture_2d<f32>>(kHouseTexture));
    }
}
```
