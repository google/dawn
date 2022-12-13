fn pow_bc91ed() {
  const arg_0 = vec2(1.0);
  const arg_1 = vec2(1.0);
  var res = pow(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  pow_bc91ed();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  pow_bc91ed();
}

@compute @workgroup_size(1)
fn compute_main() {
  pow_bc91ed();
}
