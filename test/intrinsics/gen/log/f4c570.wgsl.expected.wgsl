fn log_f4c570() {
  var res : vec3<f32> = log(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  log_f4c570();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  log_f4c570();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  log_f4c570();
}
