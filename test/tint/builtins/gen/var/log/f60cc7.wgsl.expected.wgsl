fn log_f60cc7() {
  const arg_0 = vec2(1.0);
  var res = log(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  log_f60cc7();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  log_f60cc7();
}

@compute @workgroup_size(1)
fn compute_main() {
  log_f60cc7();
}
