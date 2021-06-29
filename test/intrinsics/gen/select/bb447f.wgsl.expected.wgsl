fn select_bb447f() {
  var res : vec2<i32> = select(vec2<i32>(), vec2<i32>(), bool());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  select_bb447f();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  select_bb447f();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  select_bb447f();
}
