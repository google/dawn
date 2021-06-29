fn select_cb9301() {
  var res : vec2<bool> = select(vec2<bool>(), vec2<bool>(), vec2<bool>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  select_cb9301();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  select_cb9301();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  select_cb9301();
}
