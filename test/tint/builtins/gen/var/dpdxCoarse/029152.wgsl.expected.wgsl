fn dpdxCoarse_029152() {
  var arg_0 = 1.0;
  var res : f32 = dpdxCoarse(arg_0);
}

@stage(fragment)
fn fragment_main() {
  dpdxCoarse_029152();
}
