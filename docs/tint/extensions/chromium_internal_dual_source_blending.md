# Chromium Internal Dual Source Blending

The `chromium_internal_dual_source_blending` extension adds support for specifying dual-source blending outputs in fragment shaders.
This is useful in combination with `wgpu::BlendFactor` enum values to do hardware blending for Porter-Duff gradients between others.

## Status

Dual source blending support in Tint is for internal use in Chromium and subprojects.
It follows the agreed-upon design decided by the WebGPU W3C group but cannot be a stable extension until the `dual-source-blending` is added to the main WebGPU spec.

## Pseudo-specification

A new attribute `@blend_src(N)` (N = 0 or 1) is added that can be specified on fragment shader outputs.
It can only be used on outputs for `@location(0)` and if one of the outputs for `@location(0)` has an `@blend_src` then they must all have one.

## Example usages

```
enable chromium_internal_dual_source_blending;

struct FragInput {
  @location(0) a : vec4<f32>,
  @location(1) b : vec4<f32>,
};

struct FragOutput {
  @location(0) @blend_src(0) color : vec4<f32>,
  @location(0) @blend_src(1) blend : vec4<f32>,
};

@fragment
fn frag_main(in : FragInput) -> FragOutput {
  var output : FragOutput;
  output.color = in.a;
  output.blend = in.b;
  return output;
}
```
