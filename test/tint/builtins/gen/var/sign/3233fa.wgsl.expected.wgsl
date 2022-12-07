fn sign_3233fa() {
  var arg_0 = 1i;
  var res : i32 = sign(arg_0);
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
