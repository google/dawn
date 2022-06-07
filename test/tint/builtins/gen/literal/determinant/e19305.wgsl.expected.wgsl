fn determinant_e19305() {
  var res : f32 = determinant(mat2x2<f32>());
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
