fn max_453e04() {
  var res : vec4<u32> = max(vec4<u32>(), vec4<u32>());
}

[[stage(vertex)]]
fn vertex_main() {
  max_453e04();
}

[[stage(fragment)]]
fn fragment_main() {
  max_453e04();
}

[[stage(compute)]]
fn compute_main() {
  max_453e04();
}
