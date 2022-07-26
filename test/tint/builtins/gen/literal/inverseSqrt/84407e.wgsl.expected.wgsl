fn inverseSqrt_84407e() {
  var res : f32 = inverseSqrt(1.0f);
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
