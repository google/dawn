fn determinant_e19305() {
  var res : f32 = determinant(mat2x2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  determinant_e19305();
}

[[stage(fragment)]]
fn fragment_main() {
  determinant_e19305();
}

[[stage(compute)]]
fn compute_main() {
  determinant_e19305();
}
