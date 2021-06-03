fn faceForward_e6908b() {
  var res : vec2<f32> = faceForward(vec2<f32>(), vec2<f32>(), vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  faceForward_e6908b();
}

[[stage(fragment)]]
fn fragment_main() {
  faceForward_e6908b();
}

[[stage(compute)]]
fn compute_main() {
  faceForward_e6908b();
}
