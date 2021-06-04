fn max_ce7c30() {
  var res : i32 = max(1, 1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  max_ce7c30();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  max_ce7c30();
}

[[stage(compute)]]
fn compute_main() {
  max_ce7c30();
}
