fn cos_16dc15() {
  var res : vec3<f32> = cos(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  cos_16dc15();
}

[[stage(fragment)]]
fn fragment_main() {
  cos_16dc15();
}

[[stage(compute)]]
fn compute_main() {
  cos_16dc15();
}
