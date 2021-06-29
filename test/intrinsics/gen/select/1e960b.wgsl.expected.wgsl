fn select_1e960b() {
  var res : vec2<u32> = select(vec2<u32>(), vec2<u32>(), vec2<bool>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  select_1e960b();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  select_1e960b();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  select_1e960b();
}
