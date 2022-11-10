fn sinh_9c1092() {
  const arg_0 = vec2(1.0);
  var res = sinh(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sinh_9c1092();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sinh_9c1092();
}

@compute @workgroup_size(1)
fn compute_main() {
  sinh_9c1092();
}
