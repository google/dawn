fn reverseBits_e1f4c1() {
  var res : vec2<u32> = reverseBits(vec2<u32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  reverseBits_e1f4c1();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  reverseBits_e1f4c1();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  reverseBits_e1f4c1();
}
