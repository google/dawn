fn exp_771fd2() {
  var res : f32 = exp(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  exp_771fd2();
}

[[stage(fragment)]]
fn fragment_main() {
  exp_771fd2();
}

[[stage(compute)]]
fn compute_main() {
  exp_771fd2();
}
