fn isNan_e4978e() {
  var res : bool = isNan(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  isNan_e4978e();
}

[[stage(fragment)]]
fn fragment_main() {
  isNan_e4978e();
}

[[stage(compute)]]
fn compute_main() {
  isNan_e4978e();
}
