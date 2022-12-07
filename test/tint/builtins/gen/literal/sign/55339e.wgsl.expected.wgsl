fn sign_55339e() {
  var res = sign(vec3(1));
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
