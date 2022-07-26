fn modf_f5f20d() {
  var arg_0 = vec2<f32>(1.0f);
  var res = modf(arg_0);
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
