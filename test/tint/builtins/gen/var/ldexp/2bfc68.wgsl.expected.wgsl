fn ldexp_2bfc68() {
  const arg_0 = vec2(1.0);
  var arg_1 = vec2<i32>(1i);
  var res = ldexp(arg_0, arg_1);
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
