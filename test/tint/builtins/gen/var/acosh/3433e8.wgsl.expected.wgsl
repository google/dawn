fn acosh_3433e8() {
  const arg_0 = 1.54308063479999990619;
  var res = acosh(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acosh_3433e8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acosh_3433e8();
}

@compute @workgroup_size(1)
fn compute_main() {
  acosh_3433e8();
}
