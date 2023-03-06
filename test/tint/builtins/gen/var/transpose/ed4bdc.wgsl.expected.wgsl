fn transpose_ed4bdc() {
  var arg_0 = mat3x2<f32>(1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
  var res : mat2x3<f32> = transpose(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : mat2x3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  transpose_ed4bdc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  transpose_ed4bdc();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_ed4bdc();
}
