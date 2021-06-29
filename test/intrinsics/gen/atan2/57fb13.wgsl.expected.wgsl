fn atan2_57fb13() {
  var res : vec2<f32> = atan2(vec2<f32>(), vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  atan2_57fb13();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  atan2_57fb13();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atan2_57fb13();
}
