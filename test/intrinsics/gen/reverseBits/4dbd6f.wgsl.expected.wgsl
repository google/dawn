fn reverseBits_4dbd6f() {
  var res : vec4<i32> = reverseBits(vec4<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  reverseBits_4dbd6f();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  reverseBits_4dbd6f();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  reverseBits_4dbd6f();
}
