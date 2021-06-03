fn dpdxFine_f401a2() {
  var res : f32 = dpdxFine(1.0);
}

[[stage(fragment)]]
fn fragment_main() {
  dpdxFine_f401a2();
}
