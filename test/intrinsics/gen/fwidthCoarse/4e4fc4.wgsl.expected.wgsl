fn fwidthCoarse_4e4fc4() {
  var res : vec4<f32> = fwidthCoarse(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  fwidthCoarse_4e4fc4();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  fwidthCoarse_4e4fc4();
}

[[stage(compute)]]
fn compute_main() {
  fwidthCoarse_4e4fc4();
}
