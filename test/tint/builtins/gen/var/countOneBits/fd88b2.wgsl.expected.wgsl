@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn countOneBits_fd88b2() -> i32 {
  var arg_0 = 1i;
  var res : i32 = countOneBits(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = countOneBits_fd88b2();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = countOneBits_fd88b2();
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
  out.prevent_dce = countOneBits_fd88b2();
  return out;
}
