fn sign_d065d8() {
  var arg_0 = vec2<f32>(1.0f);
  var res : vec2<f32> = sign(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sign_d065d8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sign_d065d8();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_d065d8();
}
