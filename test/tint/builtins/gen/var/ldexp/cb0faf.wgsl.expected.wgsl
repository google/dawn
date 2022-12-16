fn ldexp_cb0faf() {
  const arg_0 = vec4(1.0);
  const arg_1 = vec4(1);
  var res = ldexp(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_cb0faf();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_cb0faf();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_cb0faf();
}
