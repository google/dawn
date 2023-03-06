fn fwidthCoarse_1e59d9() {
  var res : vec3<f32> = fwidthCoarse(vec3<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@fragment
fn fragment_main() {
  fwidthCoarse_1e59d9();
}
