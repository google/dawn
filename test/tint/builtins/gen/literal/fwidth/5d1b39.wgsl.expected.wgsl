fn fwidth_5d1b39() {
  var res : vec3<f32> = fwidth(vec3<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@fragment
fn fragment_main() {
  fwidth_5d1b39();
}
