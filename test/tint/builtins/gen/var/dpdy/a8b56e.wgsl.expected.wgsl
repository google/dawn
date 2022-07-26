fn dpdy_a8b56e() {
  var arg_0 = vec2<f32>(1.0f);
  var res : vec2<f32> = dpdy(arg_0);
}

@fragment
fn fragment_main() {
  dpdy_a8b56e();
}
