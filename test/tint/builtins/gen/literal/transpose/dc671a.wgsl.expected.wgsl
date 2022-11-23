fn transpose_dc671a() {
  var res = transpose(mat4x4(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  transpose_dc671a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  transpose_dc671a();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_dc671a();
}
