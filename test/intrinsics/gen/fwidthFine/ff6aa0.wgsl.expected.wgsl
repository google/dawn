fn fwidthFine_ff6aa0() {
  var res : vec2<f32> = fwidthFine(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  fwidthFine_ff6aa0();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  fwidthFine_ff6aa0();
}

[[stage(compute)]]
fn compute_main() {
  fwidthFine_ff6aa0();
}
