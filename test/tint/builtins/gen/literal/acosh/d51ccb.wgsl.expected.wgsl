fn acosh_d51ccb() {
  var res : vec4<f32> = acosh(vec4<f32>(1.0f));
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
