SKIP: FAILED


fn select_c31f9e() {
  var res : bool = select(bool(), bool(), bool());
}

[[stage(vertex)]]
fn vertex_main() {
  select_c31f9e();
}

[[stage(fragment)]]
fn fragment_main() {
  select_c31f9e();
}

[[stage(compute)]]
fn compute_main() {
  select_c31f9e();
}

Failed to generate: error: select not supported in HLSL backend yet
