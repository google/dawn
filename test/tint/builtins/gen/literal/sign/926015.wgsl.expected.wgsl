fn sign_926015() {
  var res : vec2<i32> = sign(vec2<i32>(1i));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sign_926015();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sign_926015();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_926015();
}
