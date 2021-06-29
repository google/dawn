fn select_b04721() {
  var res : vec3<u32> = select(vec3<u32>(), vec3<u32>(), bool());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  select_b04721();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  select_b04721();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  select_b04721();
}
