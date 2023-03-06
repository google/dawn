fn transpose_31d679() {
  var res : mat2x2<f32> = transpose(mat2x2<f32>(1.0f, 1.0f, 1.0f, 1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : mat2x2<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  transpose_31d679();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  transpose_31d679();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_31d679();
}
