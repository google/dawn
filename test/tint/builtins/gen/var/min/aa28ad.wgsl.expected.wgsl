fn min_aa28ad() {
  var arg_0 = vec2<f32>();
  var arg_1 = vec2<f32>();
  var res : vec2<f32> = min(arg_0, arg_1);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_aa28ad();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  min_aa28ad();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  min_aa28ad();
}
