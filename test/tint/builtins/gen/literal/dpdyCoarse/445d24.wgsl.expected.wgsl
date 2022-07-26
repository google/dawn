fn dpdyCoarse_445d24() {
  var res : vec4<f32> = dpdyCoarse(vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  dpdyCoarse_445d24();
}
