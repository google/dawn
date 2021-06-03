fn all_bd2dba() {
  var res : bool = all(vec3<bool>());
}

[[stage(vertex)]]
fn vertex_main() {
  all_bd2dba();
}

[[stage(fragment)]]
fn fragment_main() {
  all_bd2dba();
}

[[stage(compute)]]
fn compute_main() {
  all_bd2dba();
}
