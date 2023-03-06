fn atanh_7997d8() {
  var arg_0 = 0.5f;
  var res : f32 = atanh(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atanh_7997d8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atanh_7997d8();
}

@compute @workgroup_size(1)
fn compute_main() {
  atanh_7997d8();
}
