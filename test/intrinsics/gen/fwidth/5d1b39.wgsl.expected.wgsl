fn fwidth_5d1b39() {
  var res : vec3<f32> = fwidth(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  fwidth_5d1b39();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  fwidth_5d1b39();
}

[[stage(compute)]]
fn compute_main() {
  fwidth_5d1b39();
}
