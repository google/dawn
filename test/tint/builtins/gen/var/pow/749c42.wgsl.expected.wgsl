fn pow_749c42() {
  const arg_0 = 1.0;
  const arg_1 = 1.0;
  var res = pow(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  pow_749c42();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  pow_749c42();
}

@compute @workgroup_size(1)
fn compute_main() {
  pow_749c42();
}
