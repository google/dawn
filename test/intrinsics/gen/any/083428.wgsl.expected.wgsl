fn any_083428() {
  var res : bool = any(vec4<bool>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  any_083428();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  any_083428();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  any_083428();
}
