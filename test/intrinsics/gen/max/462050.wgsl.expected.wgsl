fn max_462050() {
  var res : vec2<f32> = max(vec2<f32>(), vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  max_462050();
}

[[stage(fragment)]]
fn fragment_main() {
  max_462050();
}

[[stage(compute)]]
fn compute_main() {
  max_462050();
}
