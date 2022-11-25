fn faceForward_b42ef3() {
  var res = faceForward(vec2(1.0), vec2(1.0), vec2(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  faceForward_b42ef3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  faceForward_b42ef3();
}

@compute @workgroup_size(1)
fn compute_main() {
  faceForward_b42ef3();
}
