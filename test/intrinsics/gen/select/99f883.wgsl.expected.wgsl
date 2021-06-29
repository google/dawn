fn select_99f883() {
  var res : u32 = select(1u, 1u, bool());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  select_99f883();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  select_99f883();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  select_99f883();
}
