fn atan2_96057c() {
  var res : f32 = atan2(1.0, 1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  atan2_96057c();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  atan2_96057c();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atan2_96057c();
}
