fn min_3941e1() {
  var res : vec4<i32> = min(vec4<i32>(), vec4<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  min_3941e1();
}

[[stage(fragment)]]
fn fragment_main() {
  min_3941e1();
}

[[stage(compute)]]
fn compute_main() {
  min_3941e1();
}
