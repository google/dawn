fn ldexp_376938() {
  const arg_0 = vec4(1.0);
  var arg_1 = vec4<i32>(1i);
  var res = ldexp(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_376938();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_376938();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_376938();
}
