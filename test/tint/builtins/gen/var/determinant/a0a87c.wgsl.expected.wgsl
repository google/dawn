fn determinant_a0a87c() {
  var arg_0 = mat4x4<f32>();
  var res : f32 = determinant(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  determinant_a0a87c();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  determinant_a0a87c();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  determinant_a0a87c();
}
