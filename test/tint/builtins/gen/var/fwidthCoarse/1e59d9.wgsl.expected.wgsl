fn fwidthCoarse_1e59d9() {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = fwidthCoarse(arg_0);
}

@fragment
fn fragment_main() {
  fwidthCoarse_1e59d9();
}
