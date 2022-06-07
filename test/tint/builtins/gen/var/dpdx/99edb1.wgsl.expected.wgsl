fn dpdx_99edb1() {
  var arg_0 = vec2<f32>();
  var res : vec2<f32> = dpdx(arg_0);
}

@fragment
fn fragment_main() {
  dpdx_99edb1();
}
