fn log_b2ce28() {
  var arg_0 = vec2<f32>(1.0f);
  var res : vec2<f32> = log(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  log_b2ce28();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  log_b2ce28();
}

@compute @workgroup_size(1)
fn compute_main() {
  log_b2ce28();
}
