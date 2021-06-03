fn isFinite_426f9f() {
  var res : bool = isFinite(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  isFinite_426f9f();
}

[[stage(fragment)]]
fn fragment_main() {
  isFinite_426f9f();
}

[[stage(compute)]]
fn compute_main() {
  isFinite_426f9f();
}
