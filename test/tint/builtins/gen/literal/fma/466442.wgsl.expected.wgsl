fn fma_466442() {
  var res = fma(1.0, 1.0, 1.0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fma_466442();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fma_466442();
}

@compute @workgroup_size(1)
fn compute_main() {
  fma_466442();
}
