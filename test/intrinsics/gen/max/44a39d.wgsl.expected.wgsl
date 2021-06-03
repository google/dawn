fn max_44a39d() {
  var res : f32 = max(1.0, 1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  max_44a39d();
}

[[stage(fragment)]]
fn fragment_main() {
  max_44a39d();
}

[[stage(compute)]]
fn compute_main() {
  max_44a39d();
}
