fn faceForward_5afbd5() {
  var res : vec3<f32> = faceForward(vec3<f32>(1.0f), vec3<f32>(1.0f), vec3<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  faceForward_5afbd5();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  faceForward_5afbd5();
}

@compute @workgroup_size(1)
fn compute_main() {
  faceForward_5afbd5();
}
