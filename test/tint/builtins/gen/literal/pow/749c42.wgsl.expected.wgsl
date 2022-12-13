fn pow_749c42() {
  var res = pow(1.0, 1.0);
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
