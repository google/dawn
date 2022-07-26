fn dpdy_feb40f() {
  var res : vec3<f32> = dpdy(vec3<f32>(1.0f));
}

@fragment
fn fragment_main() {
  dpdy_feb40f();
}
