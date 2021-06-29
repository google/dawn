fn transpose_2585cd() {
  var res : mat3x4<f32> = transpose(mat4x3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  transpose_2585cd();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  transpose_2585cd();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  transpose_2585cd();
}
