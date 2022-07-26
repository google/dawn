fn determinant_e19305() {
  var arg_0 = mat2x2<f32>(1.0f, 1.0f, 1.0f, 1.0f);
  var res : f32 = determinant(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  determinant_e19305();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  determinant_e19305();
}

@compute @workgroup_size(1)
fn compute_main() {
  determinant_e19305();
}
