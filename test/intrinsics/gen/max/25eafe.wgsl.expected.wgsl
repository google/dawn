fn max_25eafe() {
  var res : vec3<i32> = max(vec3<i32>(), vec3<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  max_25eafe();
}

[[stage(fragment)]]
fn fragment_main() {
  max_25eafe();
}

[[stage(compute)]]
fn compute_main() {
  max_25eafe();
}
