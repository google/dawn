fn transpose_2585cd() {
  var arg_0 = mat4x3<f32>(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
  var res : mat3x4<f32> = transpose(arg_0);
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
