fn transpose_d8f8ba() {
  var arg_0 = mat3x4<f32>(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
  var res : mat4x3<f32> = transpose(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  transpose_d8f8ba();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  transpose_d8f8ba();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_d8f8ba();
}
