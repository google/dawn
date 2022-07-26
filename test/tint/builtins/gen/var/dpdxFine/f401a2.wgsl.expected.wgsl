fn dpdxFine_f401a2() {
  var arg_0 = 1.0f;
  var res : f32 = dpdxFine(arg_0);
}

@fragment
fn fragment_main() {
  dpdxFine_f401a2();
}
