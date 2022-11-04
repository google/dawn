fn clamp_9d731c() {
  const arg_0 = vec2(1.0);
  const arg_1 = vec2(1.0);
  const arg_2 = vec2(1.0);
  var res = clamp(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_9d731c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_9d731c();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_9d731c();
}
