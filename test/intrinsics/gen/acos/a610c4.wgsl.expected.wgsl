fn acos_a610c4() {
  var res : vec3<f32> = acos(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  acos_a610c4();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  acos_a610c4();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  acos_a610c4();
}
