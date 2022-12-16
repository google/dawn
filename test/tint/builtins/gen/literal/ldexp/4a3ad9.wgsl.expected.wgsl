fn ldexp_4a3ad9() {
  var res = ldexp(vec3(1.0), vec3(1));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_4a3ad9();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_4a3ad9();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_4a3ad9();
}
