fn fwidthCoarse_159c8a() {
  var res : f32 = fwidthCoarse(1.0f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@fragment
fn fragment_main() {
  fwidthCoarse_159c8a();
}
