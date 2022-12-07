fn sign_55339e() {
  const arg_0 = vec3(1);
  var res = sign(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sign_55339e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sign_55339e();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_55339e();
}
