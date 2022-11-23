fn log_697e1d() {
  var res = log(vec4(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  log_697e1d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  log_697e1d();
}

@compute @workgroup_size(1)
fn compute_main() {
  log_697e1d();
}
