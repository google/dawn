fn normalize_64d8c0() {
  var res : vec3<f32> = normalize(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  normalize_64d8c0();
}

[[stage(fragment)]]
fn fragment_main() {
  normalize_64d8c0();
}

[[stage(compute)]]
fn compute_main() {
  normalize_64d8c0();
}
