fn cos_16dc15() {
  var res : vec3<f32> = cos(vec3<f32>(0.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cos_16dc15();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cos_16dc15();
}

@compute @workgroup_size(1)
fn compute_main() {
  cos_16dc15();
}
