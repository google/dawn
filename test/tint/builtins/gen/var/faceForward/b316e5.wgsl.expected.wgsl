fn faceForward_b316e5() {
  var arg_0 = vec4<f32>(1.0f);
  var arg_1 = vec4<f32>(1.0f);
  var arg_2 = vec4<f32>(1.0f);
  var res : vec4<f32> = faceForward(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  faceForward_b316e5();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  faceForward_b316e5();
}

@compute @workgroup_size(1)
fn compute_main() {
  faceForward_b316e5();
}
