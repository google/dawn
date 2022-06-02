fn atan_ad96e4() {
  var arg_0 = vec2<f32>();
  var res : vec2<f32> = atan(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan_ad96e4();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  atan_ad96e4();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  atan_ad96e4();
}
