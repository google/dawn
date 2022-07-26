fn sign_d065d8() {
  var res : vec2<f32> = sign(vec2<f32>(1.0f));
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
