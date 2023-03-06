fn log2_4036ed() {
  var res : f32 = log2(1.0f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  log2_4036ed();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  log2_4036ed();
}

@compute @workgroup_size(1)
fn compute_main() {
  log2_4036ed();
}
