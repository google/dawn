fn fwidthFine_ff6aa0() {
  var res : vec2<f32> = fwidthFine(vec2<f32>());
}

[[stage(fragment)]]
fn fragment_main() {
  fwidthFine_ff6aa0();
}
