fn sinh_7bb598() {
  var res : f32 = sinh(1.0f);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sinh_7bb598();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sinh_7bb598();
}

@compute @workgroup_size(1)
fn compute_main() {
  sinh_7bb598();
}
