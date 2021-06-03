fn sinh_7bb598() {
  var res : f32 = sinh(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  sinh_7bb598();
}

[[stage(fragment)]]
fn fragment_main() {
  sinh_7bb598();
}

[[stage(compute)]]
fn compute_main() {
  sinh_7bb598();
}
