fn dpdy_a8b56e() {
  var res : vec2<f32> = dpdy(vec2<f32>(1.0f));
}

@fragment
fn fragment_main() {
  dpdy_a8b56e();
}
