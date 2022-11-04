fn clamp_9d731c() {
  var res = clamp(vec2(1.0), vec2(1.0), vec2(1.0));
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
