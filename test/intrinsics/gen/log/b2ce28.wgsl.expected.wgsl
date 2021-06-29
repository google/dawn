fn log_b2ce28() {
  var res : vec2<f32> = log(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  log_b2ce28();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  log_b2ce28();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  log_b2ce28();
}
