@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba16uint, read_write>;

fn textureLoad_5b0f5b() -> vec4<u32> {
  var arg_1 = vec3<i32>(1i);
  var res : vec4<u32> = textureLoad(arg_0, arg_1);
  return res;
}

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@fragment
fn fragment_main() {
  prevent_dce = textureLoad_5b0f5b();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureLoad_5b0f5b();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec4<u32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = textureLoad_5b0f5b();
  return out;
}
