fn select_cb9301() {
  var arg_0 = vec2<bool>();
  var arg_1 = vec2<bool>();
  var arg_2 = vec2<bool>();
  var res : vec2<bool> = select(arg_0, arg_1, arg_2);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_cb9301();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  select_cb9301();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  select_cb9301();
}
