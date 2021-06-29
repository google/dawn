fn sign_159665() {
  var res : vec3<f32> = sign(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  sign_159665();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  sign_159665();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  sign_159665();
}
