fn select_3c25ce() {
  var res : vec3<bool> = select(vec3<bool>(), vec3<bool>(), bool());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  select_3c25ce();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  select_3c25ce();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  select_3c25ce();
}
