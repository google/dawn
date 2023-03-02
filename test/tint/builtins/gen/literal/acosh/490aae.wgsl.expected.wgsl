fn acosh_490aae() {
  var res = acosh(vec4(1.54308063479999990619));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acosh_490aae();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acosh_490aae();
}

@compute @workgroup_size(1)
fn compute_main() {
  acosh_490aae();
}
