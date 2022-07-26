fn fwidth_b83ebb() {
  var arg_0 = vec2<f32>(1.0f);
  var res : vec2<f32> = fwidth(arg_0);
}

@fragment
fn fragment_main() {
  fwidth_b83ebb();
}
