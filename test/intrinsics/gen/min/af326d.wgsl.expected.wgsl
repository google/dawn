fn min_af326d() {
  var res : f32 = min(1.0, 1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  min_af326d();
}

[[stage(fragment)]]
fn fragment_main() {
  min_af326d();
}

[[stage(compute)]]
fn compute_main() {
  min_af326d();
}
