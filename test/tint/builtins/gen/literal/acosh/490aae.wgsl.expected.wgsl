fn acosh_490aae() {
  var res = acosh(vec4(2.0));
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
