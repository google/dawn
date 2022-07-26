fn clamp_2bde41() {
  var res : vec4<f32> = clamp(vec4<f32>(1.0f), vec4<f32>(1.0f), vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_2bde41();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_2bde41();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_2bde41();
}
