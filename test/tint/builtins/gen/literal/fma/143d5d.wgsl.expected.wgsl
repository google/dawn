fn fma_143d5d() {
  var res = fma(vec4(1.0), vec4(1.0), vec4(1.0));
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
