fn dpdxFine_f92fb6() {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = dpdxFine(arg_0);
}

@fragment
fn fragment_main() {
  dpdxFine_f92fb6();
}
