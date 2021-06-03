fn tanh_c15fdb() {
  var res : f32 = tanh(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  tanh_c15fdb();
}

[[stage(fragment)]]
fn fragment_main() {
  tanh_c15fdb();
}

[[stage(compute)]]
fn compute_main() {
  tanh_c15fdb();
}
