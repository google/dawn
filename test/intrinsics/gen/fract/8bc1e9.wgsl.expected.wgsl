fn fract_8bc1e9() {
  var res : vec4<f32> = fract(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  fract_8bc1e9();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  fract_8bc1e9();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  fract_8bc1e9();
}
