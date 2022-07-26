fn dpdxCoarse_029152() {
  var arg_0 = 1.0f;
  var res : f32 = dpdxCoarse(arg_0);
}

@fragment
fn fragment_main() {
  dpdxCoarse_029152();
}
