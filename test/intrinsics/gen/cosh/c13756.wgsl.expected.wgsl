fn cosh_c13756() {
  var res : vec2<f32> = cosh(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  cosh_c13756();
}

[[stage(fragment)]]
fn fragment_main() {
  cosh_c13756();
}

[[stage(compute)]]
fn compute_main() {
  cosh_c13756();
}
