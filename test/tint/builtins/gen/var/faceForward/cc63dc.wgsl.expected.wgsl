enable f16;

fn faceForward_cc63dc() {
  var arg_0 = vec4<f16>(f16());
  var arg_1 = vec4<f16>(f16());
  var arg_2 = vec4<f16>(f16());
  var res : vec4<f16> = faceForward(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  faceForward_cc63dc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  faceForward_cc63dc();
}

@compute @workgroup_size(1)
fn compute_main() {
  faceForward_cc63dc();
}
