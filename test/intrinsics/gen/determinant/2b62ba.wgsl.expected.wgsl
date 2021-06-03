fn determinant_2b62ba() {
  var res : f32 = determinant(mat3x3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  determinant_2b62ba();
}

[[stage(fragment)]]
fn fragment_main() {
  determinant_2b62ba();
}

[[stage(compute)]]
fn compute_main() {
  determinant_2b62ba();
}
