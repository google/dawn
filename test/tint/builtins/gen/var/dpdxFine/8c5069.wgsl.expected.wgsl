fn dpdxFine_8c5069() {
  var arg_0 = vec4<f32>();
  var res : vec4<f32> = dpdxFine(arg_0);
}

@stage(fragment)
fn fragment_main() {
  dpdxFine_8c5069();
}
