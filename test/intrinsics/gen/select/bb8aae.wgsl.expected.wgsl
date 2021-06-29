fn select_bb8aae() {
  var res : vec4<f32> = select(vec4<f32>(), vec4<f32>(), vec4<bool>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  select_bb8aae();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  select_bb8aae();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  select_bb8aae();
}
