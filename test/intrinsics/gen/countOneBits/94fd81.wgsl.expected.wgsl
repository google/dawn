fn countOneBits_94fd81() {
  var res : vec2<u32> = countOneBits(vec2<u32>());
}

[[stage(vertex)]]
fn vertex_main() {
  countOneBits_94fd81();
}

[[stage(fragment)]]
fn fragment_main() {
  countOneBits_94fd81();
}

[[stage(compute)]]
fn compute_main() {
  countOneBits_94fd81();
}
