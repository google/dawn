fn select_e3e028() {
  var res : vec4<bool> = select(vec4<bool>(), vec4<bool>(), vec4<bool>());
}

[[stage(vertex)]]
fn vertex_main() {
  select_e3e028();
}

[[stage(fragment)]]
fn fragment_main() {
  select_e3e028();
}

[[stage(compute)]]
fn compute_main() {
  select_e3e028();
}
