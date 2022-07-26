fn dpdx_e263de() {
  var arg_0 = 1.0f;
  var res : f32 = dpdx(arg_0);
}

@fragment
fn fragment_main() {
  dpdx_e263de();
}
