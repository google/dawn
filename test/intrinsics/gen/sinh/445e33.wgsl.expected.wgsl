fn sinh_445e33() {
  var res : vec4<f32> = sinh(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  sinh_445e33();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  sinh_445e33();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  sinh_445e33();
}
