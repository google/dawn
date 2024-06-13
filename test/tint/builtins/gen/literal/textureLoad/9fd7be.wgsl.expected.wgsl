@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba32uint, read_write>;

fn textureLoad_9fd7be() -> vec4<u32> {
  var res : vec4<u32> = textureLoad(arg_0, vec2<i32>(1i), 1i);
  return res;
}

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@fragment
fn fragment_main() {
  prevent_dce = textureLoad_9fd7be();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureLoad_9fd7be();
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
  out.prevent_dce = textureLoad_9fd7be();
  return out;
}
