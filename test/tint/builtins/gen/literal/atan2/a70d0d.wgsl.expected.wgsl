fn atan2_a70d0d() {
  var res : vec3<f32> = atan2(vec3<f32>(1.0f), vec3<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan2_a70d0d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan2_a70d0d();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan2_a70d0d();
}
