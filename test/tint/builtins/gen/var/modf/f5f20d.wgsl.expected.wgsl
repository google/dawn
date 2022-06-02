fn modf_f5f20d() {
  var arg_0 = vec2<f32>();
  var res = modf(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  modf_f5f20d();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  modf_f5f20d();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  modf_f5f20d();
}
