enable f16;

fn sinh_69cce2() {
  var res : f16 = sinh(f16());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sinh_69cce2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sinh_69cce2();
}

@compute @workgroup_size(1)
fn compute_main() {
  sinh_69cce2();
}
