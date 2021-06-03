fn min_82b28f() {
  var res : vec2<u32> = min(vec2<u32>(), vec2<u32>());
}

[[stage(vertex)]]
fn vertex_main() {
  min_82b28f();
}

[[stage(fragment)]]
fn fragment_main() {
  min_82b28f();
}

[[stage(compute)]]
fn compute_main() {
  min_82b28f();
}
