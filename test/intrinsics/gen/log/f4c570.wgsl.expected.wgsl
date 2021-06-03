fn log_f4c570() {
  var res : vec3<f32> = log(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  log_f4c570();
}

[[stage(fragment)]]
fn fragment_main() {
  log_f4c570();
}

[[stage(compute)]]
fn compute_main() {
  log_f4c570();
}
