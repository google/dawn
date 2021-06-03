fn sign_159665() {
  var res : vec3<f32> = sign(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  sign_159665();
}

[[stage(fragment)]]
fn fragment_main() {
  sign_159665();
}

[[stage(compute)]]
fn compute_main() {
  sign_159665();
}
