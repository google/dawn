fn atan2_96057c() {
  var arg_0 = 1.0f;
  var arg_1 = 1.0f;
  var res : f32 = atan2(arg_0, arg_1);
}

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
