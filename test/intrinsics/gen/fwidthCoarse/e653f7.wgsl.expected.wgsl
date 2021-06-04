fn fwidthCoarse_e653f7() {
  var res : vec2<f32> = fwidthCoarse(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  fwidthCoarse_e653f7();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  fwidthCoarse_e653f7();
}

[[stage(compute)]]
fn compute_main() {
  fwidthCoarse_e653f7();
}
