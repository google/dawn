fn countOneBits_0f7980() {
  var res : vec4<i32> = countOneBits(vec4<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  countOneBits_0f7980();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  countOneBits_0f7980();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  countOneBits_0f7980();
}
