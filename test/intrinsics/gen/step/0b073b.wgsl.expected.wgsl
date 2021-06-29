fn step_0b073b() {
  var res : f32 = step(1.0, 1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  step_0b073b();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  step_0b073b();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  step_0b073b();
}
