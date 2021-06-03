fn log_7114a6() {
  var res : f32 = log(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  log_7114a6();
}

[[stage(fragment)]]
fn fragment_main() {
  log_7114a6();
}

[[stage(compute)]]
fn compute_main() {
  log_7114a6();
}
