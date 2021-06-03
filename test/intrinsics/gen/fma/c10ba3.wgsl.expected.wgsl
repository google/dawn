fn fma_c10ba3() {
  var res : f32 = fma(1.0, 1.0, 1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  fma_c10ba3();
}

[[stage(fragment)]]
fn fragment_main() {
  fma_c10ba3();
}

[[stage(compute)]]
fn compute_main() {
  fma_c10ba3();
}
