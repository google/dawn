fn clamp_a2de25() {
  var res : u32 = clamp(1u, 1u, 1u);
}

[[stage(vertex)]]
fn vertex_main() {
  clamp_a2de25();
}

[[stage(fragment)]]
fn fragment_main() {
  clamp_a2de25();
}

[[stage(compute)]]
fn compute_main() {
  clamp_a2de25();
}
