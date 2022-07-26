fn sinh_7bb598() {
  var arg_0 = 1.0f;
  var res : f32 = sinh(arg_0);
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
