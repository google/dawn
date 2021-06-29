fn sin_b78c91() {
  var res : f32 = sin(1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  sin_b78c91();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  sin_b78c91();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  sin_b78c91();
}
