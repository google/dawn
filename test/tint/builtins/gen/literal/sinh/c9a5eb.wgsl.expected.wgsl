fn sinh_c9a5eb() {
  var res : vec3<f32> = sinh(vec3<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sinh_c9a5eb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sinh_c9a5eb();
}

@compute @workgroup_size(1)
fn compute_main() {
  sinh_c9a5eb();
}
