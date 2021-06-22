fn fwidthCoarse_4e4fc4() {
  var res : vec4<f32> = fwidthCoarse(vec4<f32>());
}

[[stage(fragment)]]
fn fragment_main() {
  fwidthCoarse_4e4fc4();
}
