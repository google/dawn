fn fwidth_b83ebb() {
  var res : vec2<f32> = fwidth(vec2<f32>());
}

[[stage(fragment)]]
fn fragment_main() {
  fwidth_b83ebb();
}
