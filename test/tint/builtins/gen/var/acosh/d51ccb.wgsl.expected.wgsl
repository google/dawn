fn acosh_d51ccb() {
  var arg_0 = vec4<f32>(1.0f);
  var res : vec4<f32> = acosh(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acosh_d51ccb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acosh_d51ccb();
}

@compute @workgroup_size(1)
fn compute_main() {
  acosh_d51ccb();
}
