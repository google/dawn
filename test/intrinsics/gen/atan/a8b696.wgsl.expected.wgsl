fn atan_a8b696() {
  var res : vec4<f32> = atan(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  atan_a8b696();
}

[[stage(fragment)]]
fn fragment_main() {
  atan_a8b696();
}

[[stage(compute)]]
fn compute_main() {
  atan_a8b696();
}
