fn sin_4e3979() {
  var res : vec4<f32> = sin(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  sin_4e3979();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  sin_4e3979();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  sin_4e3979();
}
