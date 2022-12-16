fn ldexp_2bfc68() {
  var res = ldexp(vec2(1.0), vec2<i32>(1i));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_2bfc68();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_2bfc68();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_2bfc68();
}
