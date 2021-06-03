fn all_f46790() {
  var res : bool = all(vec2<bool>());
}

[[stage(vertex)]]
fn vertex_main() {
  all_f46790();
}

[[stage(fragment)]]
fn fragment_main() {
  all_f46790();
}

[[stage(compute)]]
fn compute_main() {
  all_f46790();
}
