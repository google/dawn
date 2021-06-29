fn transpose_4ce359() {
  var res : mat4x2<f32> = transpose(mat2x4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  transpose_4ce359();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  transpose_4ce359();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  transpose_4ce359();
}
