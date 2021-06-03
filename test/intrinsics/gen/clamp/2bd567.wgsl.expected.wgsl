fn clamp_2bd567() {
  var res : f32 = clamp(1.0, 1.0, 1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  clamp_2bd567();
}

[[stage(fragment)]]
fn fragment_main() {
  clamp_2bd567();
}

[[stage(compute)]]
fn compute_main() {
  clamp_2bd567();
}
