fn log2_5b464b() {
  const arg_0 = 1.0;
  var res = log2(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  log2_5b464b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  log2_5b464b();
}

@compute @workgroup_size(1)
fn compute_main() {
  log2_5b464b();
}
