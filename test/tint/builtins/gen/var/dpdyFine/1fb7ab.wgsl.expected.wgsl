fn dpdyFine_1fb7ab() {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = dpdyFine(arg_0);
}

@fragment
fn fragment_main() {
  dpdyFine_1fb7ab();
}
