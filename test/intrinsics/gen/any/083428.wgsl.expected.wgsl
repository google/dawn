fn any_083428() {
  var res : bool = any(vec4<bool>());
}

[[stage(vertex)]]
fn vertex_main() {
  any_083428();
}

[[stage(fragment)]]
fn fragment_main() {
  any_083428();
}

[[stage(compute)]]
fn compute_main() {
  any_083428();
}
