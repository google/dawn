fn acosh_9f213e() {
  var res = acosh(vec3(1.54308063479999990619));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acosh_9f213e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acosh_9f213e();
}

@compute @workgroup_size(1)
fn compute_main() {
  acosh_9f213e();
}
