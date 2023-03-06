fn dpdx_e263de() {
  var arg_0 = 1.0f;
  var res : f32 = dpdx(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@fragment
fn fragment_main() {
  dpdx_e263de();
}
