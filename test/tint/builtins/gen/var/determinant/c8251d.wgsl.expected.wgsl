fn determinant_c8251d() {
  const arg_0 = mat3x3(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
  var res = determinant(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  determinant_c8251d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  determinant_c8251d();
}

@compute @workgroup_size(1)
fn compute_main() {
  determinant_c8251d();
}
