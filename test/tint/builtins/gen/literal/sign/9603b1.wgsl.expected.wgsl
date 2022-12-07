fn sign_9603b1() {
  var res : vec3<i32> = sign(vec3<i32>(1i));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sign_9603b1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sign_9603b1();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_9603b1();
}
