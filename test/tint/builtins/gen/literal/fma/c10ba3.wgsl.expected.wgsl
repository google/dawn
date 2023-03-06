fn fma_c10ba3() {
  var res : f32 = fma(1.0f, 1.0f, 1.0f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fma_c10ba3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fma_c10ba3();
}

@compute @workgroup_size(1)
fn compute_main() {
  fma_c10ba3();
}
