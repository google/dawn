fn fma_143d5d() {
  const arg_0 = vec4(1.0);
  const arg_1 = vec4(1.0);
  const arg_2 = vec4(1.0);
  var res = fma(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fma_143d5d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fma_143d5d();
}

@compute @workgroup_size(1)
fn compute_main() {
  fma_143d5d();
}
