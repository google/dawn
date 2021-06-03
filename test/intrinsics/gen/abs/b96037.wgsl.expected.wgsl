fn abs_b96037() {
  var res : f32 = abs(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  abs_b96037();
}

[[stage(fragment)]]
fn fragment_main() {
  abs_b96037();
}

[[stage(compute)]]
fn compute_main() {
  abs_b96037();
}
