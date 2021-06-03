fn atan2_a70d0d() {
  var res : vec3<f32> = atan2(vec3<f32>(), vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  atan2_a70d0d();
}

[[stage(fragment)]]
fn fragment_main() {
  atan2_a70d0d();
}

[[stage(compute)]]
fn compute_main() {
  atan2_a70d0d();
}
