fn atan2_c19683() {
  const arg_0 = vec2(1.0);
  const arg_1 = vec2(1.0);
  var res = atan2(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan2_c19683();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan2_c19683();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan2_c19683();
}
