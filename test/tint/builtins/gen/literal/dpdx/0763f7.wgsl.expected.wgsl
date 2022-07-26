fn dpdx_0763f7() {
  var res : vec3<f32> = dpdx(vec3<f32>(1.0f));
}

@fragment
fn fragment_main() {
  dpdx_0763f7();
}
