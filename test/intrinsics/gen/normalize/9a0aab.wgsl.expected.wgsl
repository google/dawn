fn normalize_9a0aab() {
  var res : vec4<f32> = normalize(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  normalize_9a0aab();
}

[[stage(fragment)]]
fn fragment_main() {
  normalize_9a0aab();
}

[[stage(compute)]]
fn compute_main() {
  normalize_9a0aab();
}
