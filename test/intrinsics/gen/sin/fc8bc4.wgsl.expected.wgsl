fn sin_fc8bc4() {
  var res : vec2<f32> = sin(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  sin_fc8bc4();
}

[[stage(fragment)]]
fn fragment_main() {
  sin_fc8bc4();
}

[[stage(compute)]]
fn compute_main() {
  sin_fc8bc4();
}
