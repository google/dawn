fn sinh_c9a5eb() {
  var res : vec3<f32> = sinh(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  sinh_c9a5eb();
}

[[stage(fragment)]]
fn fragment_main() {
  sinh_c9a5eb();
}

[[stage(compute)]]
fn compute_main() {
  sinh_c9a5eb();
}
