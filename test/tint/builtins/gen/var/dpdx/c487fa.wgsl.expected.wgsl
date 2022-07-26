fn dpdx_c487fa() {
  var arg_0 = vec4<f32>(1.0f);
  var res : vec4<f32> = dpdx(arg_0);
}

@fragment
fn fragment_main() {
  dpdx_c487fa();
}
