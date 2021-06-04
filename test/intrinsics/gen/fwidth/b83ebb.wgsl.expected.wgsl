fn fwidth_b83ebb() {
  var res : vec2<f32> = fwidth(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  fwidth_b83ebb();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  fwidth_b83ebb();
}

[[stage(compute)]]
fn compute_main() {
  fwidth_b83ebb();
}
