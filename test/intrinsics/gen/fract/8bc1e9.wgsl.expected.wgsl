fn fract_8bc1e9() {
  var res : vec4<f32> = fract(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  fract_8bc1e9();
}

[[stage(fragment)]]
fn fragment_main() {
  fract_8bc1e9();
}

[[stage(compute)]]
fn compute_main() {
  fract_8bc1e9();
}
