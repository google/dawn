fn fma_466442() {
  const arg_0 = 1.0;
  const arg_1 = 1.0;
  const arg_2 = 1.0;
  var res = fma(arg_0, arg_1, arg_2);
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
