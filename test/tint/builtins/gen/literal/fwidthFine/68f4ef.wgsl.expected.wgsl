fn fwidthFine_68f4ef() {
  var res : vec4<f32> = fwidthFine(vec4<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@fragment
fn fragment_main() {
  fwidthFine_68f4ef();
}
