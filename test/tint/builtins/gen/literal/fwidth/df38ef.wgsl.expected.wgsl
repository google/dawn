fn fwidth_df38ef() {
  var res : f32 = fwidth(1.0f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@fragment
fn fragment_main() {
  fwidth_df38ef();
}
