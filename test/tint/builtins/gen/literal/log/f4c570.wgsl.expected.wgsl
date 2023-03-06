fn log_f4c570() {
  var res : vec3<f32> = log(vec3<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  log_f4c570();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  log_f4c570();
}

@compute @workgroup_size(1)
fn compute_main() {
  log_f4c570();
}
