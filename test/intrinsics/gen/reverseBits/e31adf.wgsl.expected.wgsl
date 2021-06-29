fn reverseBits_e31adf() {
  var res : u32 = reverseBits(1u);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  reverseBits_e31adf();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  reverseBits_e31adf();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  reverseBits_e31adf();
}
