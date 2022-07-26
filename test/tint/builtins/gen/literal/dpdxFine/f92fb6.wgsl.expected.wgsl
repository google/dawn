fn dpdxFine_f92fb6() {
  var res : vec3<f32> = dpdxFine(vec3<f32>(1.0f));
}

@fragment
fn fragment_main() {
  dpdxFine_f92fb6();
}
