fn fwidth_5d1b39() {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = fwidth(arg_0);
}

@fragment
fn fragment_main() {
  fwidth_5d1b39();
}
