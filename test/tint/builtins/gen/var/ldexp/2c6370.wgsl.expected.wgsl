fn ldexp_2c6370() {
  const arg_0 = vec2(1.0);
  const arg_1 = vec2(1);
  var res = ldexp(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_2c6370();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_2c6370();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_2c6370();
}
