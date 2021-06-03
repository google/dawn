fn acos_dfc915() {
  var res : vec2<f32> = acos(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  acos_dfc915();
}

[[stage(fragment)]]
fn fragment_main() {
  acos_dfc915();
}

[[stage(compute)]]
fn compute_main() {
  acos_dfc915();
}
