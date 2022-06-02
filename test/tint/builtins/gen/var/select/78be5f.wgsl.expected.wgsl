fn select_78be5f() {
  var arg_0 = vec3<f32>();
  var arg_1 = vec3<f32>();
  var arg_2 = bool();
  var res : vec3<f32> = select(arg_0, arg_1, arg_2);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_78be5f();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  select_78be5f();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  select_78be5f();
}
