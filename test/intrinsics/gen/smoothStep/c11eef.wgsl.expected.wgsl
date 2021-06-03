fn smoothStep_c11eef() {
  var res : vec2<f32> = smoothStep(vec2<f32>(), vec2<f32>(), vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  smoothStep_c11eef();
}

[[stage(fragment)]]
fn fragment_main() {
  smoothStep_c11eef();
}

[[stage(compute)]]
fn compute_main() {
  smoothStep_c11eef();
}
