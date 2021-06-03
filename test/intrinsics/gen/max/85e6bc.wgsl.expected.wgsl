fn max_85e6bc() {
  var res : vec4<i32> = max(vec4<i32>(), vec4<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  max_85e6bc();
}

[[stage(fragment)]]
fn fragment_main() {
  max_85e6bc();
}

[[stage(compute)]]
fn compute_main() {
  max_85e6bc();
}
