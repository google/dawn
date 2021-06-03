fn exp2_dea523() {
  var res : f32 = exp2(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  exp2_dea523();
}

[[stage(fragment)]]
fn fragment_main() {
  exp2_dea523();
}

[[stage(compute)]]
fn compute_main() {
  exp2_dea523();
}
