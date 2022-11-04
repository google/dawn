fn atan2_c19683() {
  var res = atan2(vec2(1.0), vec2(1.0));
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
