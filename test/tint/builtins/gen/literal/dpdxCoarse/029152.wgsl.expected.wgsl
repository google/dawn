fn dpdxCoarse_029152() {
  var res : f32 = dpdxCoarse(1.0);
}

@fragment
fn fragment_main() {
  dpdxCoarse_029152();
}
