fn dpdyCoarse_3e1ab4() {
  var res : vec2<f32> = dpdyCoarse(vec2<f32>(1.0f));
}

@fragment
fn fragment_main() {
  dpdyCoarse_3e1ab4();
}
