@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba16snorm, read>;

fn textureLoad_b0551f() -> vec4<f32> {
  var arg_1 = 1i;
  var res : vec4<f32> = textureLoad(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureLoad_b0551f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureLoad_b0551f();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = textureLoad_b0551f();
  return out;
}
