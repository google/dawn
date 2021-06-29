fn select_c31f9e() {
  var res : bool = select(bool(), bool(), bool());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  select_c31f9e();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  select_c31f9e();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  select_c31f9e();
}
