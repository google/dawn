fn sin_b78c91() {
  var res : f32 = sin(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  sin_b78c91();
}

[[stage(fragment)]]
fn fragment_main() {
  sin_b78c91();
}

[[stage(compute)]]
fn compute_main() {
  sin_b78c91();
}
