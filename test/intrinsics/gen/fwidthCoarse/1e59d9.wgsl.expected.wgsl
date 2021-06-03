fn fwidthCoarse_1e59d9() {
  var res : vec3<f32> = fwidthCoarse(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  fwidthCoarse_1e59d9();
}

[[stage(fragment)]]
fn fragment_main() {
  fwidthCoarse_1e59d9();
}

[[stage(compute)]]
fn compute_main() {
  fwidthCoarse_1e59d9();
}
