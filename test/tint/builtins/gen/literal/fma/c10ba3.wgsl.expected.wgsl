fn fma_c10ba3() {
  var res : f32 = fma(1.0, 1.0, 1.0);
}

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
