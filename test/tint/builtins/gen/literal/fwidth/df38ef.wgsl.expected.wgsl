fn fwidth_df38ef() -> f32 {
  var res : f32 = fwidth(1.0f);
  return res;
}

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

@fragment
fn fragment_main() {
  prevent_dce = fwidth_df38ef();
}
