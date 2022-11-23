fn log_b8088d() {
  var res = log(vec3(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  log_b8088d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  log_b8088d();
}

@compute @workgroup_size(1)
fn compute_main() {
  log_b8088d();
}
