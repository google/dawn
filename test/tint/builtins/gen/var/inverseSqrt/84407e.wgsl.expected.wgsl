fn inverseSqrt_84407e() {
  var arg_0 = 1.0f;
  var res : f32 = inverseSqrt(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  inverseSqrt_84407e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  inverseSqrt_84407e();
}

@compute @workgroup_size(1)
fn compute_main() {
  inverseSqrt_84407e();
}
