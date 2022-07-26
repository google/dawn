fn dpdyFine_df33aa() {
  var res : vec2<f32> = dpdyFine(vec2<f32>(1.0f));
}

@fragment
fn fragment_main() {
  dpdyFine_df33aa();
}
