fn determinant_cefdf3() {
  var res = determinant(mat4x4(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  determinant_cefdf3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  determinant_cefdf3();
}

@compute @workgroup_size(1)
fn compute_main() {
  determinant_cefdf3();
}
