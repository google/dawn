requires texel_buffers;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@group(1) @binding(0) var arg_0 : texel_buffer<r32sint, read>;

fn textureLoad_467353() -> vec4<i32> {
  var res : vec4<i32> = textureLoad(arg_0, 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureLoad_467353();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureLoad_467353();
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
  out.prevent_dce = textureLoad_467353();
  return out;
}
