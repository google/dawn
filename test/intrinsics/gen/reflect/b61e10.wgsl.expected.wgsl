fn reflect_b61e10() {
  var res : vec2<f32> = reflect(vec2<f32>(), vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  reflect_b61e10();
}

[[stage(fragment)]]
fn fragment_main() {
  reflect_b61e10();
}

[[stage(compute)]]
fn compute_main() {
  reflect_b61e10();
}
