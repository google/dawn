fn fwidth_df38ef() {
  var res : f32 = fwidth(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  fwidth_df38ef();
}

[[stage(fragment)]]
fn fragment_main() {
  fwidth_df38ef();
}

[[stage(compute)]]
fn compute_main() {
  fwidth_df38ef();
}
