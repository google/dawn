# Multisampled Array Textures

The `multisampled_array_textures` language feature adds the
`texture_multisampled_2d_array<T>` WGSL type. The texel type `T` must be `f32`, `i32`, or `u32`.

## Status

This is an experimental language feature based on the
[multisampled array textures proposal](https://github.com/gpuweb/gpuweb/blob/main/proposals/multisampled-array-textures.md).

## Syntax

The `requires` directive documents the module's dependency on the language feature. It does not
enable the feature; the implementation must support it.

```wgsl
requires multisampled_array_textures;

@group(0) @binding(0)
var texture : texture_multisampled_2d_array<f32>;
```

The language feature adds overloads for:

- `textureDimensions(texture)`
- `textureNumLayers(texture)`
- `textureNumSamples(texture)`
- `textureLoad(texture, coordinates, array_index, sample_index)`

For example:

```wgsl
requires multisampled_array_textures;

@group(0) @binding(0)
var texture : texture_multisampled_2d_array<f32>;

fn loadSample(coordinates : vec2<i32>, layer : i32, sample : i32) -> vec4<f32> {
    return textureLoad(texture, coordinates, layer, sample);
}
```
