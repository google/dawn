fn fwidthCoarse_159c8a() {
  var res : f32 = fwidthCoarse(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  fwidthCoarse_159c8a();
}

[[stage(fragment)]]
fn fragment_main() {
  fwidthCoarse_159c8a();
}

[[stage(compute)]]
fn compute_main() {
  fwidthCoarse_159c8a();
}
