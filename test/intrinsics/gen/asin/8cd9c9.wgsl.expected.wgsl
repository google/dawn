fn asin_8cd9c9() {
  var res : vec3<f32> = asin(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  asin_8cd9c9();
}

[[stage(fragment)]]
fn fragment_main() {
  asin_8cd9c9();
}

[[stage(compute)]]
fn compute_main() {
  asin_8cd9c9();
}
