fn min_0dc614() {
  var res : vec4<u32> = min(vec4<u32>(), vec4<u32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  min_0dc614();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  min_0dc614();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  min_0dc614();
}
