fn log_b2ce28() {
  var res : vec2<f32> = log(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  log_b2ce28();
}

[[stage(fragment)]]
fn fragment_main() {
  log_b2ce28();
}

[[stage(compute)]]
fn compute_main() {
  log_b2ce28();
}
