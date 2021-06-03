fn atan_02979a() {
  var res : f32 = atan(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  atan_02979a();
}

[[stage(fragment)]]
fn fragment_main() {
  atan_02979a();
}

[[stage(compute)]]
fn compute_main() {
  atan_02979a();
}
