fn log_b2ce28() {
  var res : vec2<f32> = log(vec2<f32>(1.0f));
}

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
