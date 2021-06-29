fn determinant_2b62ba() {
  var res : f32 = determinant(mat3x3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  determinant_2b62ba();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  determinant_2b62ba();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  determinant_2b62ba();
}
