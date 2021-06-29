fn cosh_c13756() {
  var res : vec2<f32> = cosh(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  cosh_c13756();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  cosh_c13756();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  cosh_c13756();
}
