fn faceForward_fc994b() {
  var res : f32 = faceForward(1.0, 1.0, 1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  faceForward_fc994b();
}

[[stage(fragment)]]
fn fragment_main() {
  faceForward_fc994b();
}

[[stage(compute)]]
fn compute_main() {
  faceForward_fc994b();
}
