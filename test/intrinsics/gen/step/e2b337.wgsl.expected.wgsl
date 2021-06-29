fn step_e2b337() {
  var res : vec4<f32> = step(vec4<f32>(), vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  step_e2b337();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  step_e2b337();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  step_e2b337();
}
