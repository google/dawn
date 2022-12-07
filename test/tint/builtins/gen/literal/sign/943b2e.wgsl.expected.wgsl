fn sign_943b2e() {
  var res = sign(vec2(1));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sign_943b2e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sign_943b2e();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_943b2e();
}
