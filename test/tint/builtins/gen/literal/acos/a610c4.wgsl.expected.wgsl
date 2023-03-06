fn acos_a610c4() {
  var res : vec3<f32> = acos(vec3<f32>(0.96891242265701293945f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acos_a610c4();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acos_a610c4();
}

@compute @workgroup_size(1)
fn compute_main() {
  acos_a610c4();
}
