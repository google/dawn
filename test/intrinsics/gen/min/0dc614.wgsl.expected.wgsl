fn min_0dc614() {
  var res : vec4<u32> = min(vec4<u32>(), vec4<u32>());
}

[[stage(vertex)]]
fn vertex_main() {
  min_0dc614();
}

[[stage(fragment)]]
fn fragment_main() {
  min_0dc614();
}

[[stage(compute)]]
fn compute_main() {
  min_0dc614();
}
