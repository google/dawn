fn reverseBits_7c4269() {
  var res : i32 = reverseBits(1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  reverseBits_7c4269();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  reverseBits_7c4269();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  reverseBits_7c4269();
}
