fn clamp_96e56a() {
  const arg_0 = 1;
  const arg_1 = 1;
  const arg_2 = 1;
  var res = clamp(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_96e56a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_96e56a();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_96e56a();
}
