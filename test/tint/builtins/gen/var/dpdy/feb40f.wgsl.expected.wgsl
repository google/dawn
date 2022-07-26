fn dpdy_feb40f() {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = dpdy(arg_0);
}

@fragment
fn fragment_main() {
  dpdy_feb40f();
}
