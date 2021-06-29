fn select_80a9a9() {
  var res : vec3<bool> = select(vec3<bool>(), vec3<bool>(), vec3<bool>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  select_80a9a9();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  select_80a9a9();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  select_80a9a9();
}
