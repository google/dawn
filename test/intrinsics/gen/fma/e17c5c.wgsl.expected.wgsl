fn fma_e17c5c() {
  var res : vec3<f32> = fma(vec3<f32>(), vec3<f32>(), vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  fma_e17c5c();
}

[[stage(fragment)]]
fn fragment_main() {
  fma_e17c5c();
}

[[stage(compute)]]
fn compute_main() {
  fma_e17c5c();
}
