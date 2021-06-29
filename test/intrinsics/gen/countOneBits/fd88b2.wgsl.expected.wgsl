fn countOneBits_fd88b2() {
  var res : i32 = countOneBits(1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  countOneBits_fd88b2();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  countOneBits_fd88b2();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  countOneBits_fd88b2();
}
