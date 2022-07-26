fn dpdx_0763f7() {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = dpdx(arg_0);
}

@fragment
fn fragment_main() {
  dpdx_0763f7();
}
