fn isInf_7bd98f() {
  var res : bool = isInf(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  isInf_7bd98f();
}

[[stage(fragment)]]
fn fragment_main() {
  isInf_7bd98f();
}

[[stage(compute)]]
fn compute_main() {
  isInf_7bd98f();
}
