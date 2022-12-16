fn ldexp_c9d0b7() {
  var res : f32 = ldexp(1.0f, 1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_c9d0b7();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_c9d0b7();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_c9d0b7();
}
