fn fwidth_df38ef() {
  var res : f32 = fwidth(1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  fwidth_df38ef();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  fwidth_df38ef();
}

[[stage(compute)]]
fn compute_main() {
  fwidth_df38ef();
}
