fn fwidth_df38ef() {
  var res : f32 = fwidth(1.0);
}

[[stage(fragment)]]
fn fragment_main() {
  fwidth_df38ef();
}
