# Chromium Experimental Dynamic Binding

The `chromium_experimental_dynamic_binding` is an experimental extension that provides different
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
extension it would be `enable chromium_experimental_dynamic_binding`.

# Specification

## `resource_binding`

* A new `resource_binding` type is added
* A `resource_binding` cannot be passed as a function parameter. This is because there is no way
  in WGSL to currently write the needed pointer type (e.g. `ptr<handle, resource_binding, write>`).
* A `resource_binding` can not be accessed with the usual array subscript operators. e.g. the following
  is disallowed:

  ```
  var a: resource_binding;
  fn foo() {
      var b = a[0];
  }
  ```
* Two helper methods `getBinding` and `hasBinding` are provided for accessing a `resource_binding`.

### `hasBinding`
```
@must_use fn hasBinding<T>(a: resource_binding, index: I) -> bool
```
* `I` is an `i32` or `u32`
* `T` is a format-less storage texture (e.g. `texture_storage_2d<f32>`), sampled texture, multisampled
  texture, or depth texture.

`hasBinding` returns true if the item a `index` of the array is of type `T`.

### `getBinding`
```
@must_use fn getBinding<T>(a: resource_binding, index: I) -> T
```
* `I` is an `i32` or `u32`
* `T` is a format-less storage texture (e.g. `texture_storage_2d<f32>`), sampled texture, multisampled
  texture, or depth texture.

`getBinding` returns the value at `index` of type `T`.

If `index` is outside the bounds of the binding array then a default texture of type `T` will be
returned. If the item at `index` is not of type `T` then a default texture of type `T` is returned.

### `arrayLength`
```
@must_use fn arrayLength(a: resource_binding) -> u32
```
Returns the length of the array.

# Example usage

## `resource_binding`
```
enable chromium_experimental_dynamic_binding;

@group(0) @binding(0) var textures : resource_binding;

@fragment fn fs() {
    if (hasBinding<texture_2d<f32>>(textures, 0)) {
        let tex = textureLoad(getBinding<texture_2d<f32>>(textures, 0));
    }
}
```
