fn reverseBits_222177() {
  var res : vec2<i32> = reverseBits(vec2<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  reverseBits_222177();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  reverseBits_222177();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  reverseBits_222177();
}
