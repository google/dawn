fn select_ed8a15() {
  var res : i32 = select(1, 1, bool());
}

[[stage(vertex)]]
fn vertex_main() {
  select_ed8a15();
}

[[stage(fragment)]]
fn fragment_main() {
  select_ed8a15();
}

[[stage(compute)]]
fn compute_main() {
  select_ed8a15();
}
