fn fract_a49758() {
  var res : vec3<f32> = fract(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  fract_a49758();
}

[[stage(fragment)]]
fn fragment_main() {
  fract_a49758();
}

[[stage(compute)]]
fn compute_main() {
  fract_a49758();
}
