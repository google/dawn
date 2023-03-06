fn dpdx_c487fa() {
  var arg_0 = vec4<f32>(1.0f);
  var res : vec4<f32> = dpdx(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@fragment
fn fragment_main() {
  dpdx_c487fa();
}
