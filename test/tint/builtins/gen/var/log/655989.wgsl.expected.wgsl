fn log_655989() {
  const arg_0 = 1.0;
  var res = log(arg_0);
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
