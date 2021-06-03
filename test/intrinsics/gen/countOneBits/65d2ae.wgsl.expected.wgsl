fn countOneBits_65d2ae() {
  var res : vec3<i32> = countOneBits(vec3<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  countOneBits_65d2ae();
}

[[stage(fragment)]]
fn fragment_main() {
  countOneBits_65d2ae();
}

[[stage(compute)]]
fn compute_main() {
  countOneBits_65d2ae();
}
