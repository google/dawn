fn acosh_e38f5c() {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = acosh(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acosh_e38f5c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acosh_e38f5c();
}

@compute @workgroup_size(1)
fn compute_main() {
  acosh_e38f5c();
}
