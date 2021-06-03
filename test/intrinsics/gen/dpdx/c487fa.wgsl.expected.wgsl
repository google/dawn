fn dpdx_c487fa() {
  var res : vec4<f32> = dpdx(vec4<f32>());
}

[[stage(fragment)]]
fn fragment_main() {
  dpdx_c487fa();
}
