fn sign_3bdab6() {
  const arg_0 = vec4(1);
  var res = sign(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sign_3bdab6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sign_3bdab6();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_3bdab6();
}
