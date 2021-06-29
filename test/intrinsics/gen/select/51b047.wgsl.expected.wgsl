fn select_51b047() {
  var res : vec2<u32> = select(vec2<u32>(), vec2<u32>(), bool());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  select_51b047();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  select_51b047();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  select_51b047();
}
