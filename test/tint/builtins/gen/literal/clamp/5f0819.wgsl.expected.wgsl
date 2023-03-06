fn clamp_5f0819() {
  var res : vec3<i32> = clamp(vec3<i32>(1i), vec3<i32>(1i), vec3<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_5f0819();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_5f0819();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_5f0819();
}
