fn log_7114a6() {
  var arg_0 = 1.0f;
  var res : f32 = log(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  log_7114a6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  log_7114a6();
}

@compute @workgroup_size(1)
fn compute_main() {
  log_7114a6();
}
