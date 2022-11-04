fn clamp_96e56a() {
  var res = clamp(1, 1, 1);
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
