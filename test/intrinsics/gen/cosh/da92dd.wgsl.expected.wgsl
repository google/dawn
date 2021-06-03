fn cosh_da92dd() {
  var res : f32 = cosh(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  cosh_da92dd();
}

[[stage(fragment)]]
fn fragment_main() {
  cosh_da92dd();
}

[[stage(compute)]]
fn compute_main() {
  cosh_da92dd();
}
