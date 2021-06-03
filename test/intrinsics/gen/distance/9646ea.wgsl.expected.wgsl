fn distance_9646ea() {
  var res : f32 = distance(vec4<f32>(), vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  distance_9646ea();
}

[[stage(fragment)]]
fn fragment_main() {
  distance_9646ea();
}

[[stage(compute)]]
fn compute_main() {
  distance_9646ea();
}
