fn atan2_a70d0d() {
  var res : vec3<f32> = atan2(vec3<f32>(), vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  atan2_a70d0d();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  atan2_a70d0d();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atan2_a70d0d();
}
