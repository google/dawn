fn fwidthCoarse_159c8a() {
  var res : f32 = fwidthCoarse(1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  fwidthCoarse_159c8a();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  fwidthCoarse_159c8a();
}

[[stage(compute)]]
fn compute_main() {
  fwidthCoarse_159c8a();
}
