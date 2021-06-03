fn countOneBits_fd88b2() {
  var res : i32 = countOneBits(1);
}

[[stage(vertex)]]
fn vertex_main() {
  countOneBits_fd88b2();
}

[[stage(fragment)]]
fn fragment_main() {
  countOneBits_fd88b2();
}

[[stage(compute)]]
fn compute_main() {
  countOneBits_fd88b2();
}
