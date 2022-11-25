fn faceForward_fe522b() {
  var res = faceForward(vec3(1.0), vec3(1.0), vec3(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  faceForward_fe522b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  faceForward_fe522b();
}

@compute @workgroup_size(1)
fn compute_main() {
  faceForward_fe522b();
}
