fn distance_0657d4() {
  var res : f32 = distance(vec3<f32>(), vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  distance_0657d4();
}

[[stage(fragment)]]
fn fragment_main() {
  distance_0657d4();
}

[[stage(compute)]]
fn compute_main() {
  distance_0657d4();
}
