fn transpose_2585cd() {
  var res : mat3x4<f32> = transpose(mat4x3<f32>(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  transpose_2585cd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  transpose_2585cd();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_2585cd();
}
