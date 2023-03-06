fn fwidth_d2ab9a() {
  var res : vec4<f32> = fwidth(vec4<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@fragment
fn fragment_main() {
  fwidth_d2ab9a();
}
