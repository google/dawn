fn transpose_553e90() {
  var res = transpose(mat4x2(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  transpose_553e90();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  transpose_553e90();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_553e90();
}
