fn transpose_854336() {
  var arg_0 = mat3x3<f32>(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
  var res : mat3x3<f32> = transpose(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  transpose_854336();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  transpose_854336();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_854336();
}
