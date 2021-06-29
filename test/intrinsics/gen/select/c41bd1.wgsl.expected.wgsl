fn select_c41bd1() {
  var res : vec4<bool> = select(vec4<bool>(), vec4<bool>(), bool());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  select_c41bd1();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  select_c41bd1();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  select_c41bd1();
}
