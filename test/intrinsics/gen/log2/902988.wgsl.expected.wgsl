fn log2_902988() {
  var res : vec4<f32> = log2(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  log2_902988();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  log2_902988();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  log2_902988();
}
