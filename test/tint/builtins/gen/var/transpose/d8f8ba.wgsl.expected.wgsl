fn transpose_d8f8ba() {
  var arg_0 = mat3x4<f32>();
  var res : mat4x3<f32> = transpose(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  transpose_d8f8ba();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  transpose_d8f8ba();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  transpose_d8f8ba();
}
