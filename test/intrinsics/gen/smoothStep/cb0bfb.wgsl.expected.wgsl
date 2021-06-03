fn smoothStep_cb0bfb() {
  var res : f32 = smoothStep(1.0, 1.0, 1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  smoothStep_cb0bfb();
}

[[stage(fragment)]]
fn fragment_main() {
  smoothStep_cb0bfb();
}

[[stage(compute)]]
fn compute_main() {
  smoothStep_cb0bfb();
}
