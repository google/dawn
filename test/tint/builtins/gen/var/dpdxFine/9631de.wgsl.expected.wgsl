fn dpdxFine_9631de() {
  var arg_0 = vec2<f32>(1.0f);
  var res : vec2<f32> = dpdxFine(arg_0);
}

@fragment
fn fragment_main() {
  dpdxFine_9631de();
}
