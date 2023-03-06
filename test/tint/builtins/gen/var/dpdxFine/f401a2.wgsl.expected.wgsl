fn dpdxFine_f401a2() {
  var arg_0 = 1.0f;
  var res : f32 = dpdxFine(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@fragment
fn fragment_main() {
  dpdxFine_f401a2();
}
