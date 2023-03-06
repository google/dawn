fn dpdxFine_f401a2() {
  var res : f32 = dpdxFine(1.0f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@fragment
fn fragment_main() {
  dpdxFine_f401a2();
}
