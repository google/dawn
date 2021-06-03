fn fwidthFine_f1742d() {
  var res : f32 = fwidthFine(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  fwidthFine_f1742d();
}

[[stage(fragment)]]
fn fragment_main() {
  fwidthFine_f1742d();
}

[[stage(compute)]]
fn compute_main() {
  fwidthFine_f1742d();
}
