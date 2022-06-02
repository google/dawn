fn select_01e2cd() {
  var arg_0 = vec3<i32>();
  var arg_1 = vec3<i32>();
  var arg_2 = vec3<bool>();
  var res : vec3<i32> = select(arg_0, arg_1, arg_2);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_01e2cd();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  select_01e2cd();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  select_01e2cd();
}
