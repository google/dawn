fn reverseBits_a6ccd4() {
  var res : vec3<u32> = reverseBits(vec3<u32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  reverseBits_a6ccd4();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  reverseBits_a6ccd4();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  reverseBits_a6ccd4();
}
