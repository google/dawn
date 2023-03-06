fn exp_d98450() {
  var res : vec3<f32> = exp(vec3<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp_d98450();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  exp_d98450();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp_d98450();
}
