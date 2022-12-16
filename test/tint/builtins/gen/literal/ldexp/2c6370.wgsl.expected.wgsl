fn ldexp_2c6370() {
  var res = ldexp(vec2(1.0), vec2(1));
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
