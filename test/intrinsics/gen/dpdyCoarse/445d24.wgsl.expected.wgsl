fn dpdyCoarse_445d24() {
  var res : vec4<f32> = dpdyCoarse(vec4<f32>());
}

[[stage(fragment)]]
fn fragment_main() {
  dpdyCoarse_445d24();
}
