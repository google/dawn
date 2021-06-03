fn step_0b073b() {
  var res : f32 = step(1.0, 1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  step_0b073b();
}

[[stage(fragment)]]
fn fragment_main() {
  step_0b073b();
}

[[stage(compute)]]
fn compute_main() {
  step_0b073b();
}
