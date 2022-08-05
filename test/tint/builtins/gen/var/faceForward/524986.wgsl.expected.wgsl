enable f16;

fn faceForward_524986() {
  var arg_0 = vec3<f16>(f16());
  var arg_1 = vec3<f16>(f16());
  var arg_2 = vec3<f16>(f16());
  var res : vec3<f16> = faceForward(arg_0, arg_1, arg_2);
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
