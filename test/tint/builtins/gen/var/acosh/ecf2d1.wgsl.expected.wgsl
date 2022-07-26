fn acosh_ecf2d1() {
  var arg_0 = 1.0f;
  var res : f32 = acosh(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acosh_ecf2d1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acosh_ecf2d1();
}

@compute @workgroup_size(1)
fn compute_main() {
  acosh_ecf2d1();
}
