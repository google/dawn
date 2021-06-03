fn sin_01f241() {
  var res : vec3<f32> = sin(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  sin_01f241();
}

[[stage(fragment)]]
fn fragment_main() {
  sin_01f241();
}

[[stage(compute)]]
fn compute_main() {
  sin_01f241();
}
