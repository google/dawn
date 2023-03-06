fn dpdxCoarse_029152() {
  var res : f32 = dpdxCoarse(1.0f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@fragment
fn fragment_main() {
  dpdxCoarse_029152();
}
