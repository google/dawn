fn all_353d6a() {
  var res : bool = all(bool());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  all_353d6a();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  all_353d6a();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  all_353d6a();
}
