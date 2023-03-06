enable f16;

fn faceForward_cc63dc() {
  var arg_0 = vec4<f16>(1.0h);
  var arg_1 = vec4<f16>(1.0h);
  var arg_2 = vec4<f16>(1.0h);
  var res : vec4<f16> = faceForward(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

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
