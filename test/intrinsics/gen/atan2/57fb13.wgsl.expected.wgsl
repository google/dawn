fn atan2_57fb13() {
  var res : vec2<f32> = atan2(vec2<f32>(), vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  atan2_57fb13();
}

[[stage(fragment)]]
fn fragment_main() {
  atan2_57fb13();
}

[[stage(compute)]]
fn compute_main() {
  atan2_57fb13();
}
