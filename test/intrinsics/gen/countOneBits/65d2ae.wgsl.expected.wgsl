fn countOneBits_65d2ae() {
  var res : vec3<i32> = countOneBits(vec3<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  countOneBits_65d2ae();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  countOneBits_65d2ae();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  countOneBits_65d2ae();
}
