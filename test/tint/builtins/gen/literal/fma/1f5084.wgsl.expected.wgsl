fn fma_1f5084() {
  var res = fma(vec2(1.0), vec2(1.0), vec2(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fma_1f5084();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fma_1f5084();
}

@compute @workgroup_size(1)
fn compute_main() {
  fma_1f5084();
}
