fn select_ed8a15() {
  var res : i32 = select(1, 1, bool());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  select_ed8a15();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  select_ed8a15();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  select_ed8a15();
}
