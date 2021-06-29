fn faceForward_e6908b() {
  var res : vec2<f32> = faceForward(vec2<f32>(), vec2<f32>(), vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  faceForward_e6908b();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  faceForward_e6908b();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  faceForward_e6908b();
}
