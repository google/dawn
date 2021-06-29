fn faceForward_fc994b() {
  var res : f32 = faceForward(1.0, 1.0, 1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  faceForward_fc994b();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  faceForward_fc994b();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  faceForward_fc994b();
}
