fn select_ab069f() {
  var res : vec4<i32> = select(vec4<i32>(), vec4<i32>(), bool());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  select_ab069f();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  select_ab069f();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  select_ab069f();
}
