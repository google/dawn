fn dot_e994c7() {
  var res : u32 = dot(vec4<u32>(), vec4<u32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  dot_e994c7();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  dot_e994c7();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  dot_e994c7();
}
