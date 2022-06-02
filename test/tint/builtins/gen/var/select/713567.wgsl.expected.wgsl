fn select_713567() {
  var arg_0 = vec4<f32>();
  var arg_1 = vec4<f32>();
  var arg_2 = bool();
  var res : vec4<f32> = select(arg_0, arg_1, arg_2);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_713567();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  select_713567();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  select_713567();
}
