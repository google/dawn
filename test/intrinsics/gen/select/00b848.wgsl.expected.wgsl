fn select_00b848() {
  var res : vec2<i32> = select(vec2<i32>(), vec2<i32>(), vec2<bool>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  select_00b848();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  select_00b848();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  select_00b848();
}
