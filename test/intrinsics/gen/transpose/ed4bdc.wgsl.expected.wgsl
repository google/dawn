fn transpose_ed4bdc() {
  var res : mat2x3<f32> = transpose(mat3x2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  transpose_ed4bdc();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  transpose_ed4bdc();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  transpose_ed4bdc();
}
