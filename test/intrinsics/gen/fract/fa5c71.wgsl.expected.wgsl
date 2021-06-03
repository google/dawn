fn fract_fa5c71() {
  var res : f32 = fract(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  fract_fa5c71();
}

[[stage(fragment)]]
fn fragment_main() {
  fract_fa5c71();
}

[[stage(compute)]]
fn compute_main() {
  fract_fa5c71();
}
