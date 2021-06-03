fn select_a2860e() {
  var res : vec4<i32> = select(vec4<i32>(), vec4<i32>(), vec4<bool>());
}

[[stage(vertex)]]
fn vertex_main() {
  select_a2860e();
}

[[stage(fragment)]]
fn fragment_main() {
  select_a2860e();
}

[[stage(compute)]]
fn compute_main() {
  select_a2860e();
}
