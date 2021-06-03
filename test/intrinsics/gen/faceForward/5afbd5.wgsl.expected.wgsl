fn faceForward_5afbd5() {
  var res : vec3<f32> = faceForward(vec3<f32>(), vec3<f32>(), vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  faceForward_5afbd5();
}

[[stage(fragment)]]
fn fragment_main() {
  faceForward_5afbd5();
}

[[stage(compute)]]
fn compute_main() {
  faceForward_5afbd5();
}
