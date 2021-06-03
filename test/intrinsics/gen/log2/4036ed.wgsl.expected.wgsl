fn log2_4036ed() {
  var res : f32 = log2(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  log2_4036ed();
}

[[stage(fragment)]]
fn fragment_main() {
  log2_4036ed();
}

[[stage(compute)]]
fn compute_main() {
  log2_4036ed();
}
