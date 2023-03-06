fn fwidth_d2ab9a() {
  var arg_0 = vec4<f32>(1.0f);
  var res : vec4<f32> = fwidth(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@fragment
fn fragment_main() {
  fwidth_d2ab9a();
}
