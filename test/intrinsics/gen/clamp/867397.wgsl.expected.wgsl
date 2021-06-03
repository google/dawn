fn clamp_867397() {
  var res : vec3<f32> = clamp(vec3<f32>(), vec3<f32>(), vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  clamp_867397();
}

[[stage(fragment)]]
fn fragment_main() {
  clamp_867397();
}

[[stage(compute)]]
fn compute_main() {
  clamp_867397();
}
