fn clamp_d396af() {
  var res = clamp(vec4(1), vec4(1), vec4(1));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_d396af();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_d396af();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_d396af();
}
