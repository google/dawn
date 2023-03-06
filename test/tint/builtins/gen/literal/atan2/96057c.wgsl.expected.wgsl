fn atan2_96057c() {
  var res : f32 = atan2(1.0f, 1.0f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan2_96057c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan2_96057c();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan2_96057c();
}
