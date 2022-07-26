fn sqrt_20c74e() {
  var arg_0 = 1.0f;
  var res : f32 = sqrt(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sqrt_20c74e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sqrt_20c74e();
}

@compute @workgroup_size(1)
fn compute_main() {
  sqrt_20c74e();
}
