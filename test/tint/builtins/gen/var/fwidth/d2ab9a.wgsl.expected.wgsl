fn fwidth_d2ab9a() {
  var arg_0 = vec4<f32>();
  var res : vec4<f32> = fwidth(arg_0);
}

@fragment
fn fragment_main() {
  fwidth_d2ab9a();
}
