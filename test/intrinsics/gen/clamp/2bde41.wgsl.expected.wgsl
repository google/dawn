fn clamp_2bde41() {
  var res : vec4<f32> = clamp(vec4<f32>(), vec4<f32>(), vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  clamp_2bde41();
}

[[stage(fragment)]]
fn fragment_main() {
  clamp_2bde41();
}

[[stage(compute)]]
fn compute_main() {
  clamp_2bde41();
}
