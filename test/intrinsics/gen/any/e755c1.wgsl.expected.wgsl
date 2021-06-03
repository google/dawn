fn any_e755c1() {
  var res : bool = any(vec3<bool>());
}

[[stage(vertex)]]
fn vertex_main() {
  any_e755c1();
}

[[stage(fragment)]]
fn fragment_main() {
  any_e755c1();
}

[[stage(compute)]]
fn compute_main() {
  any_e755c1();
}
