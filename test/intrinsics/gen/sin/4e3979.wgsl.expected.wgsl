fn sin_4e3979() {
  var res : vec4<f32> = sin(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  sin_4e3979();
}

[[stage(fragment)]]
fn fragment_main() {
  sin_4e3979();
}

[[stage(compute)]]
fn compute_main() {
  sin_4e3979();
}
