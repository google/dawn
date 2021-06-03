fn countOneBits_0f7980() {
  var res : vec4<i32> = countOneBits(vec4<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  countOneBits_0f7980();
}

[[stage(fragment)]]
fn fragment_main() {
  countOneBits_0f7980();
}

[[stage(compute)]]
fn compute_main() {
  countOneBits_0f7980();
}
