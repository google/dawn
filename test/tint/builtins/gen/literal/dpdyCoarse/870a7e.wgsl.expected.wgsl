fn dpdyCoarse_870a7e() {
  var res : f32 = dpdyCoarse(1.0f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@fragment
fn fragment_main() {
  dpdyCoarse_870a7e();
}
