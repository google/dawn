fn reflect_feae90() {
  var res : f32 = reflect(1.0, 1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  reflect_feae90();
}

[[stage(fragment)]]
fn fragment_main() {
  reflect_feae90();
}

[[stage(compute)]]
fn compute_main() {
  reflect_feae90();
}
