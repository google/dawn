fn transpose_84a763() {
  var res = transpose(mat2x4(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  transpose_84a763();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  transpose_84a763();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_84a763();
}
