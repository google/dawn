# Chromium Experimental Dynamic Binding

The `chromium_experimental_dynamic_binding` is an experimental extension that allows using both
runtime sized `binding_array` and type-less `binding_array`. This provides the needed basis for
dynamic binding (a.k.a. bind-less).

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

This extension adds new features to the `binding_array` feature added previously with the
`sized_binding_arrays` extension [1].

## `binding_array<T>`

* A new `binding_array<T>` type is added.
* `T` is a format-less storage texture (e.g. `texture_storage_2d<f32>`), sampled texture, multisampled
  texture, or depth texture.
* This is a runtime-array version of the `binding_array<T, N>` as added by `sized_binding_arrays`.
* A runtime binding array may not be passed as a function parameter. This is because there is no way
  in WGSL to currently write the needed pointer type (e.g.
  `ptr<handle, binding_array<texture_1d<f32>>, write>`).
* An out-of-bounds read from the `binding_array<T>` will return any value from the `binding_array`.


### `arrayLength`
```
@must_use fn arrayLength(a: ptr<handle, binding_array<T>, RW>) -> u32
```

Similar to the runtime-array version, returns the length of the array.

## `binding_array`

* A new `binding_array` type is added
* This is a type-less version of `binding_array<T>`. As such, any restrictions on runtime binding
  array is relevant to `binding_array`.
* A `binding_array` can not be accessed with the usual array subscript operators. e.g. the following
  is disallowed:

  ```
  var a: binding_array;
  fn foo() {
      var b = a[0];
  }
  ```
* Two helper methods `getBinding` and `hasBinding` are provided for accessing a `binding_array`.

### `hasBinding`
```
@must_use fn hasBinding<T>(a: binding_array, index: I) -> bool
```
* `I` is an `i32` or `u32`
* `T` is a format-less storage texture (e.g. `texture_storage_2d<f32>`), sampled texture, multisampled
  texture, or depth texture.

`hasBinding` returns true if the item a `index` of the array is of type `T`.

### `getBinding`
```
@must_use fn getBinding<T>(a: binding_array, index: I) -> T
```
* `I` is an `i32` or `u32`
* `T` is a format-less storage texture (e.g. `texture_storage_2d<f32>`), sampled texture, multisampled
  texture, or depth texture.

`getBinding` returns the value at `index` of type `T`.

If `index` is outside the bounds of the binding array then a default texture of type `T` will be
returned. If the item at `index` is not of type `T` then a default texture of type `T` is returned.

### `arrayLength`
```
@must_use fn arrayLength(a: ptr<handle, binding_array, RW>) -> u32
```

Similar to the runtime-array version, returns the length of the array.

# Example usage

## `binding_array<T>`

```
enable chromium_experimental_dynamic_binding;

@group(0) @binding(0) var sampled_textures : binding_array<texture_2d<f32>>;

@fragment fn fs() {
    let texture_load = textureLoad(sampled_textures[0], vec2(0, 0), 0);
}
```

## `binding_array`

```
enable chromium_experimental_dynamic_binding;

@group(0) @binding(0) var textures : binding_array;

@fragment fn fs() {
    if (hasBinding<texture_2d<f32>>(textures, 0)) {
        let tex = textureLoad(getBinding<texture_2d<f32>>(textures, 0));
    }
}
```

# References
1.[Sized Binding Arrays](https://github.com/gpuweb/gpuweb/blob/main/proposals/sized-binding-arrays.md)
