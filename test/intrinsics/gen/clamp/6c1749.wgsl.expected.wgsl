fn clamp_6c1749() {
  var res : vec2<i32> = clamp(vec2<i32>(), vec2<i32>(), vec2<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  clamp_6c1749();
}

[[stage(fragment)]]
fn fragment_main() {
  clamp_6c1749();
}

[[stage(compute)]]
fn compute_main() {
  clamp_6c1749();
}
