fn max_4883ac() {
  var res : vec3<f32> = max(vec3<f32>(), vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  max_4883ac();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  max_4883ac();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  max_4883ac();
}
