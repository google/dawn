fn sinh_7bb598() {
  var res : f32 = sinh(1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  sinh_7bb598();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  sinh_7bb598();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  sinh_7bb598();
}
