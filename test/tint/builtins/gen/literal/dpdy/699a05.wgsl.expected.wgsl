fn dpdy_699a05() {
  var res : vec4<f32> = dpdy(vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  dpdy_699a05();
}
