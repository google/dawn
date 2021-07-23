fn modf_684d46() {
  var res = modf(1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  modf_684d46();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  modf_684d46();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  modf_684d46();
}
