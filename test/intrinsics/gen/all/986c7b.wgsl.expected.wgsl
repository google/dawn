fn all_986c7b() {
  var res : bool = all(vec4<bool>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  all_986c7b();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  all_986c7b();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  all_986c7b();
}
