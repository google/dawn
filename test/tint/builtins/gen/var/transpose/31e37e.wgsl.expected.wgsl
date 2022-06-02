fn transpose_31e37e() {
  var arg_0 = mat4x2<f32>();
  var res : mat2x4<f32> = transpose(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  transpose_31e37e();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  transpose_31e37e();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  transpose_31e37e();
}
