fn cosh_c892bb() {
  const arg_0 = 0.0;
  var res = cosh(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cosh_c892bb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cosh_c892bb();
}

@compute @workgroup_size(1)
fn compute_main() {
  cosh_c892bb();
}
