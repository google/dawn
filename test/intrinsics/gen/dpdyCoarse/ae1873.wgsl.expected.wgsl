fn dpdyCoarse_ae1873() {
  var res : vec3<f32> = dpdyCoarse(vec3<f32>());
}

[[stage(fragment)]]
fn fragment_main() {
  dpdyCoarse_ae1873();
}
