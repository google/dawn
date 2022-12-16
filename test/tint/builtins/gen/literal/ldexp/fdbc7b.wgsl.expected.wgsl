fn ldexp_fdbc7b() {
  var res = ldexp(1.0, 1);
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
