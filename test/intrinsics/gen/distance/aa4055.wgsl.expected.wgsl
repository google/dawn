fn distance_aa4055() {
  var res : f32 = distance(vec2<f32>(), vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  distance_aa4055();
}

[[stage(fragment)]]
fn fragment_main() {
  distance_aa4055();
}

[[stage(compute)]]
fn compute_main() {
  distance_aa4055();
}
