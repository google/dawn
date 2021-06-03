fn min_aa28ad() {
  var res : vec2<f32> = min(vec2<f32>(), vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  min_aa28ad();
}

[[stage(fragment)]]
fn fragment_main() {
  min_aa28ad();
}

[[stage(compute)]]
fn compute_main() {
  min_aa28ad();
}
