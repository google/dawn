fn clamp_177548() {
  const arg_0 = vec2(1);
  const arg_1 = vec2(1);
  const arg_2 = vec2(1);
  var res = clamp(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_177548();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_177548();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_177548();
}
