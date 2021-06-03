fn clamp_5f0819() {
  var res : vec3<i32> = clamp(vec3<i32>(), vec3<i32>(), vec3<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  clamp_5f0819();
}

[[stage(fragment)]]
fn fragment_main() {
  clamp_5f0819();
}

[[stage(compute)]]
fn compute_main() {
  clamp_5f0819();
}
