fn sign_f5da6a() {
  var res = sign(vec4(1.0));
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
