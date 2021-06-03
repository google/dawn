fn max_e8192f() {
  var res : vec2<i32> = max(vec2<i32>(), vec2<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  max_e8192f();
}

[[stage(fragment)]]
fn fragment_main() {
  max_e8192f();
}

[[stage(compute)]]
fn compute_main() {
  max_e8192f();
}
