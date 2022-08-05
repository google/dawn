enable f16;

fn faceForward_fb0f2e() {
  var arg_0 = vec2<f16>(f16());
  var arg_1 = vec2<f16>(f16());
  var arg_2 = vec2<f16>(f16());
  var res : vec2<f16> = faceForward(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  faceForward_fb0f2e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  faceForward_fb0f2e();
}

@compute @workgroup_size(1)
fn compute_main() {
  faceForward_fb0f2e();
}
