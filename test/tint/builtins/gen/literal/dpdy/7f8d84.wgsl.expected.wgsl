fn dpdy_7f8d84() {
  var res : f32 = dpdy(1.0f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@fragment
fn fragment_main() {
  dpdy_7f8d84();
}
