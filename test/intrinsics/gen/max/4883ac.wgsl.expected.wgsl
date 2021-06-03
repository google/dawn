fn max_4883ac() {
  var res : vec3<f32> = max(vec3<f32>(), vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  max_4883ac();
}

[[stage(fragment)]]
fn fragment_main() {
  max_4883ac();
}

[[stage(compute)]]
fn compute_main() {
  max_4883ac();
}
