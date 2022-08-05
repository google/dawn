enable f16;

fn fma_ab7818() {
  var res : vec4<f16> = fma(vec4<f16>(f16()), vec4<f16>(f16()), vec4<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fma_ab7818();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fma_ab7818();
}

@compute @workgroup_size(1)
fn compute_main() {
  fma_ab7818();
}
