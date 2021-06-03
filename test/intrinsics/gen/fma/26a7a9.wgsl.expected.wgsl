fn fma_26a7a9() {
  var res : vec2<f32> = fma(vec2<f32>(), vec2<f32>(), vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  fma_26a7a9();
}

[[stage(fragment)]]
fn fragment_main() {
  fma_26a7a9();
}

[[stage(compute)]]
fn compute_main() {
  fma_26a7a9();
}
