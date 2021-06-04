fn round_1c7897() {
  var res : vec3<f32> = round(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  round_1c7897();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  round_1c7897();
}

[[stage(compute)]]
fn compute_main() {
  round_1c7897();
}
