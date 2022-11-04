fn sign_f5da6a() {
  const arg_0 = vec4(1.0);
  var res = sign(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sign_f5da6a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sign_f5da6a();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_f5da6a();
}
