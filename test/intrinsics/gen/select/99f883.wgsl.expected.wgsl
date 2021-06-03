fn select_99f883() {
  var res : u32 = select(1u, 1u, bool());
}

[[stage(vertex)]]
fn vertex_main() {
  select_99f883();
}

[[stage(fragment)]]
fn fragment_main() {
  select_99f883();
}

[[stage(compute)]]
fn compute_main() {
  select_99f883();
}
