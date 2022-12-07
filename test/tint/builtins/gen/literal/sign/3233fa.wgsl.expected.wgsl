fn sign_3233fa() {
  var res : i32 = sign(1i);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sign_3233fa();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sign_3233fa();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_3233fa();
}
