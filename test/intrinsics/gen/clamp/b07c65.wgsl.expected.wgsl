fn clamp_b07c65() {
  var res : i32 = clamp(1, 1, 1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  clamp_b07c65();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  clamp_b07c65();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  clamp_b07c65();
}
