fn ldexp_fdbc7b() {
  const arg_0 = 1.0;
  const arg_1 = 1;
  var res = ldexp(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_fdbc7b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_fdbc7b();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_fdbc7b();
}
