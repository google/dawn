fn pow_e42f20() {
  const arg_0 = vec3(1.0);
  const arg_1 = vec3(1.0);
  var res = pow(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  pow_e42f20();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  pow_e42f20();
}

@compute @workgroup_size(1)
fn compute_main() {
  pow_e42f20();
}
