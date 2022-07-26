fn fwidthCoarse_4e4fc4() {
  var arg_0 = vec4<f32>(1.0f);
  var res : vec4<f32> = fwidthCoarse(arg_0);
}

@fragment
fn fragment_main() {
  fwidthCoarse_4e4fc4();
}
