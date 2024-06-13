@group(1) @binding(0) var arg_0 : texture_storage_2d_array<bgra8unorm, read_write>;

fn textureNumLayers_a54655() -> u32 {
  var res : u32 = textureNumLayers(arg_0);
  return res;
}

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

@fragment
fn fragment_main() {
  prevent_dce = textureNumLayers_a54655();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureNumLayers_a54655();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : u32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = textureNumLayers_a54655();
  return out;
}
