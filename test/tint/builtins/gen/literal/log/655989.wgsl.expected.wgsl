fn log_655989() {
  var res = log(1.0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  log_655989();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  log_655989();
}

@compute @workgroup_size(1)
fn compute_main() {
  log_655989();
}
