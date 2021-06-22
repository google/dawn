fn fwidth_d2ab9a() {
  var res : vec4<f32> = fwidth(vec4<f32>());
}

[[stage(fragment)]]
fn fragment_main() {
  fwidth_d2ab9a();
}
