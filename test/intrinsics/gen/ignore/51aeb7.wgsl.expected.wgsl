fn ignore_51aeb7() {
  ignore(1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  ignore_51aeb7();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  ignore_51aeb7();
}

[[stage(compute)]]
fn compute_main() {
  ignore_51aeb7();
}
