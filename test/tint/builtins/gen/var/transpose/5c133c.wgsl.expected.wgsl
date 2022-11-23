fn transpose_5c133c() {
  const arg_0 = mat4x3(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
  var res = transpose(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  transpose_5c133c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  transpose_5c133c();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_5c133c();
}
