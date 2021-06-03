fn step_19accd() {
  var res : vec2<f32> = step(vec2<f32>(), vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  step_19accd();
}

[[stage(fragment)]]
fn fragment_main() {
  step_19accd();
}

[[stage(compute)]]
fn compute_main() {
  step_19accd();
}
