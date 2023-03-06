fn determinant_2b62ba() {
  var res : f32 = determinant(mat3x3<f32>(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  determinant_2b62ba();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  determinant_2b62ba();
}

@compute @workgroup_size(1)
fn compute_main() {
  determinant_2b62ba();
}
