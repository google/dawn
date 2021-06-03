fn dpdxFine_8c5069() {
  var res : vec4<f32> = dpdxFine(vec4<f32>());
}

[[stage(fragment)]]
fn fragment_main() {
  dpdxFine_8c5069();
}
