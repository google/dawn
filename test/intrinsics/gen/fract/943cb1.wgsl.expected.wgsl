fn fract_943cb1() {
  var res : vec2<f32> = fract(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  fract_943cb1();
}

[[stage(fragment)]]
fn fragment_main() {
  fract_943cb1();
}

[[stage(compute)]]
fn compute_main() {
  fract_943cb1();
}
