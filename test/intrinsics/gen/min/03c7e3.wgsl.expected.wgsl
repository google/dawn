fn min_03c7e3() {
  var res : vec2<i32> = min(vec2<i32>(), vec2<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  min_03c7e3();
}

[[stage(fragment)]]
fn fragment_main() {
  min_03c7e3();
}

[[stage(compute)]]
fn compute_main() {
  min_03c7e3();
}
