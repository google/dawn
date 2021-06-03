fn select_cb9301() {
  var res : vec2<bool> = select(vec2<bool>(), vec2<bool>(), vec2<bool>());
}

[[stage(vertex)]]
fn vertex_main() {
  select_cb9301();
}

[[stage(fragment)]]
fn fragment_main() {
  select_cb9301();
}

[[stage(compute)]]
fn compute_main() {
  select_cb9301();
}
