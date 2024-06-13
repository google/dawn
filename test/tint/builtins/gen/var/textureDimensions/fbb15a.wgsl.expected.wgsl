@group(1) @binding(0) var arg_0 : texture_storage_2d<bgra8unorm, write>;

fn textureDimensions_fbb15a() -> vec2<u32> {
  var res : vec2<u32> = textureDimensions(arg_0);
  return res;
}

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

@fragment
fn fragment_main() {
  prevent_dce = textureDimensions_fbb15a();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureDimensions_fbb15a();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec2<u32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = textureDimensions_fbb15a();
  return out;
}
