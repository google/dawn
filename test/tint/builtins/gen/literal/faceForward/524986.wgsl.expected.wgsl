enable f16;

fn faceForward_524986() {
  var res : vec3<f16> = faceForward(vec3<f16>(f16()), vec3<f16>(f16()), vec3<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  faceForward_524986();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  faceForward_524986();
}

@compute @workgroup_size(1)
fn compute_main() {
  faceForward_524986();
}
