fn acos_489247() {
  var res : f32 = acos(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  acos_489247();
}

[[stage(fragment)]]
fn fragment_main() {
  acos_489247();
}

[[stage(compute)]]
fn compute_main() {
  acos_489247();
}
