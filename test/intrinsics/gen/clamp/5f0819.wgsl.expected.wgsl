fn clamp_5f0819() {
  var res : vec3<i32> = clamp(vec3<i32>(), vec3<i32>(), vec3<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  clamp_5f0819();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  clamp_5f0819();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  clamp_5f0819();
}
