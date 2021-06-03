fn min_a45171() {
  var res : vec3<i32> = min(vec3<i32>(), vec3<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  min_a45171();
}

[[stage(fragment)]]
fn fragment_main() {
  min_a45171();
}

[[stage(compute)]]
fn compute_main() {
  min_a45171();
}
