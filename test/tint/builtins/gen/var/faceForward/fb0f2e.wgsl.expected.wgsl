enable f16;

fn faceForward_fb0f2e() {
  var arg_0 = vec2<f16>(1.0h);
  var arg_1 = vec2<f16>(1.0h);
  var arg_2 = vec2<f16>(1.0h);
  var res : vec2<f16> = faceForward(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

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
