fn cos_c3b486() {
  var res : vec2<f32> = cos(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  cos_c3b486();
}

[[stage(fragment)]]
fn fragment_main() {
  cos_c3b486();
}

[[stage(compute)]]
fn compute_main() {
  cos_c3b486();
}
