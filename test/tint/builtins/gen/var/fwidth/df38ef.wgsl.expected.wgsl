fn fwidth_df38ef() {
  var arg_0 = 1.0f;
  var res : f32 = fwidth(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@fragment
fn fragment_main() {
  fwidth_df38ef();
}
