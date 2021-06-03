fn min_c70bb7() {
  var res : vec3<u32> = min(vec3<u32>(), vec3<u32>());
}

[[stage(vertex)]]
fn vertex_main() {
  min_c70bb7();
}

[[stage(fragment)]]
fn fragment_main() {
  min_c70bb7();
}

[[stage(compute)]]
fn compute_main() {
  min_c70bb7();
}
