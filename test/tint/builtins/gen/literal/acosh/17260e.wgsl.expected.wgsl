fn acosh_17260e() {
  var res = acosh(vec2(2.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acosh_17260e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acosh_17260e();
}

@compute @workgroup_size(1)
fn compute_main() {
  acosh_17260e();
}
