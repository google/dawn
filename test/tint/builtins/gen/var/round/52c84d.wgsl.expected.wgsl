fn round_52c84d() {
  var arg_0 = vec2<f32>();
  var res : vec2<f32> = round(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  round_52c84d();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  round_52c84d();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  round_52c84d();
}
