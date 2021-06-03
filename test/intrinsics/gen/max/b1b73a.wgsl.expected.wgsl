fn max_b1b73a() {
  var res : vec3<u32> = max(vec3<u32>(), vec3<u32>());
}

[[stage(vertex)]]
fn vertex_main() {
  max_b1b73a();
}

[[stage(fragment)]]
fn fragment_main() {
  max_b1b73a();
}

[[stage(compute)]]
fn compute_main() {
  max_b1b73a();
}
