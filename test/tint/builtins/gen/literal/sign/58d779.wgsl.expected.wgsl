fn sign_58d779() {
  var res : vec4<i32> = sign(vec4<i32>(1i));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sign_58d779();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sign_58d779();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_58d779();
}
