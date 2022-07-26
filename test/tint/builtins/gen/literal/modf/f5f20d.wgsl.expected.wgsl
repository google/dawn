fn modf_f5f20d() {
  var res = modf(vec2<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  modf_f5f20d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  modf_f5f20d();
}

@compute @workgroup_size(1)
fn compute_main() {
  modf_f5f20d();
}
