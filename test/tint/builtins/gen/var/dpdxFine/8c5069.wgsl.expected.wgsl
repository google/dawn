fn dpdxFine_8c5069() {
  var arg_0 = vec4<f32>(1.0f);
  var res : vec4<f32> = dpdxFine(arg_0);
}

@fragment
fn fragment_main() {
  dpdxFine_8c5069();
}
