fn min_aa28ad() {
  var res : vec2<f32> = min(vec2<f32>(), vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  min_aa28ad();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  min_aa28ad();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  min_aa28ad();
}
