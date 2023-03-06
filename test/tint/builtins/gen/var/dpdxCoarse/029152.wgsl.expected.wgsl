fn dpdxCoarse_029152() {
  var arg_0 = 1.0f;
  var res : f32 = dpdxCoarse(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@fragment
fn fragment_main() {
  dpdxCoarse_029152();
}
