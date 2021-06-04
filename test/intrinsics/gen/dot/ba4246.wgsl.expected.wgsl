fn dot_ba4246() {
  var res : f32 = dot(vec3<f32>(), vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  dot_ba4246();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  dot_ba4246();
}

[[stage(compute)]]
fn compute_main() {
  dot_ba4246();
}
