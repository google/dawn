fn fwidthCoarse_4e4fc4() {
  var res : vec4<f32> = fwidthCoarse(vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  fwidthCoarse_4e4fc4();
}
