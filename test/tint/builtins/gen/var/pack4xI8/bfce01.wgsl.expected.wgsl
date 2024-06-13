fn pack4xI8_bfce01() -> u32 {
  var arg_0 = vec4<i32>(1i);
  var res : u32 = pack4xI8(arg_0);
  return res;
}

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

@fragment
fn fragment_main() {
  prevent_dce = pack4xI8_bfce01();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = pack4xI8_bfce01();
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
  out.prevent_dce = pack4xI8_bfce01();
  return out;
}
