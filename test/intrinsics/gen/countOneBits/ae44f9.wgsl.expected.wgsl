fn countOneBits_ae44f9() {
  var res : u32 = countOneBits(1u);
}

[[stage(vertex)]]
fn vertex_main() {
  countOneBits_ae44f9();
}

[[stage(fragment)]]
fn fragment_main() {
  countOneBits_ae44f9();
}

[[stage(compute)]]
fn compute_main() {
  countOneBits_ae44f9();
}
