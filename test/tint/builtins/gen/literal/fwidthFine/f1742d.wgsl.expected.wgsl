fn fwidthFine_f1742d() {
  var res : f32 = fwidthFine(1.0f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@fragment
fn fragment_main() {
  fwidthFine_f1742d();
}
