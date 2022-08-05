enable f16;

fn fma_c8abb3() {
  var res : f16 = fma(f16(), f16(), f16());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fma_c8abb3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fma_c8abb3();
}

@compute @workgroup_size(1)
fn compute_main() {
  fma_c8abb3();
}
