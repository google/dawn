fn clamp_7706d7() {
  var res : vec2<u32> = clamp(vec2<u32>(), vec2<u32>(), vec2<u32>());
}

[[stage(vertex)]]
fn vertex_main() {
  clamp_7706d7();
}

[[stage(fragment)]]
fn fragment_main() {
  clamp_7706d7();
}

[[stage(compute)]]
fn compute_main() {
  clamp_7706d7();
}
