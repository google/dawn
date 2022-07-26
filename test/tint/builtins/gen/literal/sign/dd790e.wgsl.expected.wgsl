fn sign_dd790e() {
  var res : f32 = sign(1.0f);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sign_dd790e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sign_dd790e();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_dd790e();
}
