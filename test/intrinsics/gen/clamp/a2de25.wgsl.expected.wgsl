fn clamp_a2de25() {
  var res : u32 = clamp(1u, 1u, 1u);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  clamp_a2de25();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  clamp_a2de25();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  clamp_a2de25();
}
