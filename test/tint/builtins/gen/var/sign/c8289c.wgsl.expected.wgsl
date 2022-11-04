fn sign_c8289c() {
  const arg_0 = 1.0;
  var res = sign(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sign_c8289c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sign_c8289c();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_c8289c();
}
