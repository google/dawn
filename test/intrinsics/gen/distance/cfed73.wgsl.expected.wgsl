fn distance_cfed73() {
  var res : f32 = distance(1.0, 1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  distance_cfed73();
}

[[stage(fragment)]]
fn fragment_main() {
  distance_cfed73();
}

[[stage(compute)]]
fn compute_main() {
  distance_cfed73();
}
