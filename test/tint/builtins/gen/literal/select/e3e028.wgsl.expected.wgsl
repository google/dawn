@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn select_e3e028() -> i32 {
  var res : vec4<bool> = select(vec4<bool>(true), vec4<bool>(true), vec4<bool>(true));
  return select(0, 1, all((res == vec4<bool>())));
}

@fragment
fn fragment_main() {
  prevent_dce = select_e3e028();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = select_e3e028();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : i32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = select_e3e028();
  return out;
}
