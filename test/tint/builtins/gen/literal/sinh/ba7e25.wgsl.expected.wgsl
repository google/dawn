enable f16;

fn sinh_ba7e25() {
  var res : vec4<f16> = sinh(vec4<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sinh_ba7e25();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sinh_ba7e25();
}

@compute @workgroup_size(1)
fn compute_main() {
  sinh_ba7e25();
}
