fn sign_ab6301() {
  const arg_0 = vec3(1.0);
  var res = sign(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sign_ab6301();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sign_ab6301();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_ab6301();
}
