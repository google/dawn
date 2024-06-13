@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba32sint, read_write>;

fn textureLoad_1d43ae() -> vec4<i32> {
  var arg_1 = 1i;
  var res : vec4<i32> = textureLoad(arg_0, arg_1);
  return res;
}

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@fragment
fn fragment_main() {
  prevent_dce = textureLoad_1d43ae();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureLoad_1d43ae();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec4<i32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = textureLoad_1d43ae();
  return out;
}
