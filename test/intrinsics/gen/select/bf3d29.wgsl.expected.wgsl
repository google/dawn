fn select_bf3d29() {
  var res : vec2<f32> = select(vec2<f32>(), vec2<f32>(), bool());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  select_bf3d29();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  select_bf3d29();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  select_bf3d29();
}
