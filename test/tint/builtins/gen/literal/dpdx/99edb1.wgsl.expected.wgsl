fn dpdx_99edb1() {
  var res : vec2<f32> = dpdx(vec2<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@fragment
fn fragment_main() {
  dpdx_99edb1();
}
