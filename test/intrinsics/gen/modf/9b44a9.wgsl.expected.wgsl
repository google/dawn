fn modf_9b44a9() {
  var res = modf(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  modf_9b44a9();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  modf_9b44a9();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  modf_9b44a9();
}
