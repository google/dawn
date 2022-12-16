fn ldexp_71ebe3() {
  var res = ldexp(1.0, 1i);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_71ebe3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_71ebe3();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_71ebe3();
}
