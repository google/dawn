fn countOneBits_0d0e46() {
  var res : vec4<u32> = countOneBits(vec4<u32>());
}

[[stage(vertex)]]
fn vertex_main() {
  countOneBits_0d0e46();
}

[[stage(fragment)]]
fn fragment_main() {
  countOneBits_0d0e46();
}

[[stage(compute)]]
fn compute_main() {
  countOneBits_0d0e46();
}
