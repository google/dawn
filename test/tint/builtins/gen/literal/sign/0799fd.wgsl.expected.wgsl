fn sign_0799fd() {
  var res = sign(vec2(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sign_0799fd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sign_0799fd();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_0799fd();
}
