fn min_93cfc4() {
  var res : vec3<f32> = min(vec3<f32>(), vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  min_93cfc4();
}

[[stage(fragment)]]
fn fragment_main() {
  min_93cfc4();
}

[[stage(compute)]]
fn compute_main() {
  min_93cfc4();
}
