fn clamp_548fc7() {
  var res : vec3<u32> = clamp(vec3<u32>(), vec3<u32>(), vec3<u32>());
}

[[stage(vertex)]]
fn vertex_main() {
  clamp_548fc7();
}

[[stage(fragment)]]
fn fragment_main() {
  clamp_548fc7();
}

[[stage(compute)]]
fn compute_main() {
  clamp_548fc7();
}
