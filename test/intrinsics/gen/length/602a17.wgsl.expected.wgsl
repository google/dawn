fn length_602a17() {
  var res : f32 = length(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  length_602a17();
}

[[stage(fragment)]]
fn fragment_main() {
  length_602a17();
}

[[stage(compute)]]
fn compute_main() {
  length_602a17();
}
