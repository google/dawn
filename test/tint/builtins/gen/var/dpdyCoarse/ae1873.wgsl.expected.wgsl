fn dpdyCoarse_ae1873() {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = dpdyCoarse(arg_0);
}

@fragment
fn fragment_main() {
  dpdyCoarse_ae1873();
}
