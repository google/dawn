fn dpdx_0763f7() {
  var arg_0 = vec3<f32>();
  var res : vec3<f32> = dpdx(arg_0);
}

@stage(fragment)
fn fragment_main() {
  dpdx_0763f7();
}
