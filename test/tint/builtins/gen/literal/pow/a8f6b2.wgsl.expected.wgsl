fn pow_a8f6b2() {
  var res = pow(vec4(1.0), vec4(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  pow_a8f6b2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  pow_a8f6b2();
}

@compute @workgroup_size(1)
fn compute_main() {
  pow_a8f6b2();
}
