fn asin_064953() {
  var res : vec4<f32> = asin(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  asin_064953();
}

[[stage(fragment)]]
fn fragment_main() {
  asin_064953();
}

[[stage(compute)]]
fn compute_main() {
  asin_064953();
}
