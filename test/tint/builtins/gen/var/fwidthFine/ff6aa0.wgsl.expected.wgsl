fn fwidthFine_ff6aa0() {
  var arg_0 = vec2<f32>(1.0f);
  var res : vec2<f32> = fwidthFine(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@fragment
fn fragment_main() {
  fwidthFine_ff6aa0();
}
