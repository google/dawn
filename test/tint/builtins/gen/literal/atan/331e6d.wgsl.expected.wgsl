fn atan_331e6d() {
  var res : vec3<f32> = atan(vec3<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan_331e6d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan_331e6d();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan_331e6d();
}
